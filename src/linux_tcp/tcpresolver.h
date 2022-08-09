/**
 *  TcpResolver.h
 *
 *  Class that is used for the DNS lookup of the hostname of the RabbitMQ 
 *  server, and to make the initial connection
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 - 2020 Copernica BV
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
#include <netinet/in.h>
#include <poll.h>

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class TcpResolver : public TcpExtState
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
     *  Timeout for the connect call in seconds.
     *  @var int
     */
    int _timeout;
    
    /**
     *  A pipe that is used to send back the socket that is connected to RabbitMQ
     *  @var Pipe
     */
    Pipe _pipe;
    
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
     *  How should the addresses be ordered when we want to connect
     *  @var ConnectionOrdre
     */
    ConnectionOrder _order;


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
            AddressInfo addresses(_hostname.data(), _port, _order);
    
            // the pollfd structure, needed for poll()
            pollfd fd;
    
            // iterate over the addresses
            for (size_t i = 0; i < addresses.size(); ++i)
            {
                // create the socket
                _socket = socket(addresses[i]->ai_family, addresses[i]->ai_socktype, addresses[i]->ai_protocol);
                
                // move on on failure
                if (_socket < 0) continue;

                // turn socket into a non-blocking socket and set the close-on-exec bit
                fcntl(_socket, F_SETFL, O_NONBLOCK | O_CLOEXEC);
                
                // we set the 'keepalive' option so that we automatically detect if the peer is dead
                int keepalive = 1;
                
                // set the keepalive option
                setsockopt(_socket, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));

                // try to connect non-blocking
                if (connect(_socket, addresses[i]->ai_addr, addresses[i]->ai_addrlen) == 0) break;

                // set the struct members
                fd.fd = _socket;
                fd.events = POLLOUT;
                fd.revents = 0;
                
                // the return code
                int ret = 0;

                // keep looping until we get a 'final' result
                do
                {
                    // perform the poll, with a very long time to allow the event to occur
                    ret = poll(&fd, 1, _timeout * 1000);

                // we want to retry if the call was interrupted by a signal
                } while (ret == -1 && errno == EINTR);

                // log the error for the time being
                if (ret == 0) _error = "connection timed out";
                
                // otherwise, select might've failed
                else if (ret < 0) _error = strerror(errno);

                // otherwise the connect failed/succeeded
                else
                {
                    // the error
                    int err = 0;
                    socklen_t len = 4;

                    // get the options
                    getsockopt(_socket, SOL_SOCKET, SO_ERROR, &err, &len);

                    // if the error is zero, we break, socket is now valid
                    if (err == 0) break;

                    // set the error with the value
                    _error = strerror(err);
                }

                // close socket because connect failed
                ::close(_socket);
                    
                // socket no longer is valid
                _socket = -1;
            }
            
            // connection succeeded, mark socket as non-blocking
            if (_socket >= 0) 
            {
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
            
        // notify the master thread by sending a byte over the pipe, store error if this fails
        if (!_pipe.notify()) _error = strerror(errno);
    }

public:
    /**
     *  Constructor
     *  @param  parent      Parent connection object
     *  @param  hostname    The hostname for the lookup
     *  @param  portnumber  The portnumber for the lookup
     *  @param  secure      Do we need a secure tls connection when ready?
     *  @param  timeout     timeout per connection attempt
     *  @param  order       How should we oreder the addresses of the host to connect to
     */
    TcpResolver(TcpParent *parent, std::string hostname, uint16_t port, bool secure, int timeout, const ConnectionOrder &order) : 
        TcpExtState(parent), 
        _hostname(std::move(hostname)),
        _secure(secure),
        _port(port),
        _timeout(timeout),
        _order(order)
    {
        // tell the event loop to monitor the filedescriptor of the pipe
        parent->onIdle(this, _pipe.in(), readable);
        
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
        _parent->onIdle(this, _pipe.in(), 0);

        // wait for the thread to be ready
        if (_thread.joinable()) _thread.join();
    }
    
    /**
     *  Number of bytes in the outgoing buffer
     *  @return std::size_t
     */
    virtual std::size_t queued() const override { return _buffer.size(); }
    
    /**
     *  Proceed to the next state
     *  @param  monitor         object that checks if the connection still exists
     *  @return TcpState *
     */
    TcpState *proceed(const Monitor &monitor)
    {
        // prevent exceptions
        try
        {
            // the other thread must be ready by now, so we join it, which also guarantees us
            // that the memory of the two threads have been synchronized (without this call
            // it is possible that the memory of the threads have not been synchronized, and
            // _socket has not yet been set)
            _thread.join();
            
            // socket should be connected by now
            if (_socket < 0) throw std::runtime_error(_error.data());
        
            // report that the network-layer is connected
            _parent->onConnected(this);

            // handler callback might have destroyed connection
            if (!monitor.valid()) return nullptr;
            
            // if we need a secure connection, we move to the tls handshake (this could throw)
            if (_secure) return new SslHandshake(this, std::move(_hostname), std::move(_buffer));
            
            // otherwise we have a valid regular tcp connection
            else return new TcpConnected(this, std::move(_buffer));
        }
        catch (const std::runtime_error &error)
        {
            // report error
            _parent->onError(this, error.what(), false);
        
            // handler callback might have destroyed connection
            if (!monitor.valid()) return nullptr;

            // create dummy implementation
            return new TcpClosed(this);
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
