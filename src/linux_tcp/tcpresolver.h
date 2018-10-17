/**
 *  TcpResolver.h
 *
 *  Class that is used for the DNS lookup of the hostname of the RabbitMQ 
 *  server, and to make the initial connection
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 - 2018 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include "pipe.h"
#include "tcpstate.h"
#include "tcpclosed.h"
#include "tcpconnected.h"
#include "openssl.h"
#include "sslhandshake.h"
#include <thread>

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class TcpResolver : public TcpState
{
private:
    /**
     *  The hostname that we're trying to resolve
     *  @var std::string
     */
    std::string _hostname;
    
    /**
     *  Should we be using a secure connection?
     *  @var bool
     */
    bool _secure;
    
    /**
     *  The portnumber to connect to
     *  @var uint16_t
     */
    uint16_t _port;
    
    /**
     *  A pipe that is used to send back the socket that is connected to RabbitMQ
     *  @var Pipe
     */
    Pipe _pipe;
    
    /**
     *  Non-blocking socket that is connected to RabbitMQ
     *  @var int
     */
    int _socket = -1;
    
    /**
     *  Possible error that occured
     *  @var std::string
     */
    std::string _error;
    
    /**
     *  Data that was sent to the connection, while busy resolving the hostname
     *  @var TcpBuffer
     */
    TcpOutBuffer _buffer;
    
    /**
     *  Thread in which the DNS lookup occurs
     *  @var std::thread
     */
    std::thread _thread;


    /**
     *  Run the thread
     */
    void run()
    {
        // prevent exceptions
        try
        {
            // check if we support openssl in the first place
            if (_secure && !OpenSSL::valid()) throw std::runtime_error("Secure connection cannot be established: libssl.so cannot be loaded");
            
            // get address info
            AddressInfo addresses(_hostname.data(), _port);
    
            // iterate over the addresses
            for (size_t i = 0; i < addresses.size(); ++i)
            {
                // create the socket
                _socket = socket(addresses[i]->ai_family, addresses[i]->ai_socktype, addresses[i]->ai_protocol);
                
                // move on on failure
                if (_socket < -1) continue;
                
                // connect to the socket
                if (connect(_socket, addresses[i]->ai_addr, addresses[i]->ai_addrlen) == 0) break;
                
                // log the error for the time being
                _error = strerror(errno);

                // close socket because connect failed
                close(_socket);
                    
                // socket no longer is valid
                _socket = -1;
            }
            
            // connection succeeded, mark socket as non-blocking
            if (_socket >= 0) 
            {
                // turn socket into a non-blocking socket and set the close-on-exec bit
                fcntl(_socket, F_SETFL, O_NONBLOCK | O_CLOEXEC);
                
                // we want to enable "nodelay" on sockets (otherwise all send operations are s-l-o-w
                int optval = 1;
                
                // set the option
                setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(int));

#ifdef AMQP_CPP_USE_SO_NOSIGPIPE
                set_sockopt_nosigpipe(_socket);
#endif
            }
        }
        catch (const std::runtime_error &error)
        {
            // address could not be resolved, we ignore this for now, but store the error
            _error = error.what();
        }
            
        // notify the master thread by sending a byte over the pipe
        _pipe.notify();
    }

public:
    /**
     *  Constructor
     *  @param  connection  Parent connection object
     *  @param  hostname    The hostname for the lookup
     *  @param  portnumber  The portnumber for the lookup
     *  @param  secure      Do we need a secure tls connection when ready?
     *  @param  handler     User implemented handler object
     */
    TcpResolver(TcpConnection *connection, const std::string &hostname, uint16_t port, bool secure, TcpHandler *handler) : 
        TcpState(connection, handler), 
        _hostname(hostname),
        _secure(secure),
        _port(port)
    {
        // tell the event loop to monitor the filedescriptor of the pipe
        handler->monitor(connection, _pipe.in(), readable);
        
        // we can now start the thread (must be started after filedescriptor is monitored!)
        std::thread thread(std::bind(&TcpResolver::run, this));
        
        // store thread in member
        _thread.swap(thread);
    }
    
    /**
     *  Destructor
     */
    virtual ~TcpResolver() noexcept
    {
        // stop monitoring the pipe filedescriptor
        _handler->monitor(_connection, _pipe.in(), 0);

        // wait for the thread to be ready
        _thread.join();
    }
    
    /**
     *  Number of bytes in the outgoing buffer
     *  @return std::size_t
     */
    virtual std::size_t queued() const override { return _buffer.size(); }
    
    /**
     *  Proceed to the next state
     *  @return TcpState *
     */
    TcpState *proceed(const Monitor &monitor)
    {
        // do we have a valid socket?
        if (_socket >= 0) 
        {
            // if we need a secure connection, we move to the tls handshake
            // @todo catch possible exception
            if (_secure) return new SslHandshake(_connection, _socket, _hostname, std::move(_buffer), _handler);
            
            // otherwise we have a valid regular tcp connection
            return new TcpConnected(_connection, _socket, std::move(_buffer), _handler);
        }
        else
        {
            // report error
            _handler->onError(_connection, _error.data());
        
            // handler callback might have destroyed connection
            if (!monitor.valid()) return nullptr;

            // create dummy implementation
            return new TcpClosed(_connection, _handler);
        }
    }
    
    /**
     *  Wait for the resolver to be ready
     *  @param  monitor     Object to check if connection still exists
     *  @param  fd          The filedescriptor that is active
     *  @param  flags       Flags to indicate that fd is readable and/or writable
     *  @return             New implementation object
     */
    virtual TcpState *process(const Monitor &monitor, int fd, int flags) override
    {
        // only works if the incoming pipe is readable
        if (fd != _pipe.in() || !(flags & readable)) return this;

        // proceed to the next state
        return proceed(monitor);
    }
    
    /**
     *  Flush state / wait for the connection to complete
     *  @param  monitor     Object to check if connection still exists
     *  @return             New implementation object
     */
    virtual TcpState *flush(const Monitor &monitor) override
    {
        // just wait for the other thread to be ready
        _thread.join();
        
        // proceed to the next state
        return proceed(monitor);
    }

    /**
     *  Send data over the connection
     *  @param  buffer      buffer to send
     *  @param  size        size of the buffer
     */
    virtual void send(const char *buffer, size_t size) override
    {
        // add data to buffer
        _buffer.add(buffer, size);
    }
};

/**
 *  End of namespace
 */
}
