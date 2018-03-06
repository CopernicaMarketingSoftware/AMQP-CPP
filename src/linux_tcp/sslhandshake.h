/**
 *  SslHandshake.h
 *
 *  Implementation of the TCP state that is responsible for setting
 *  up the STARTTLS handshake.
 *
 *  @copyright 2018 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include "tcpoutbuffer.h"
#include "sslconnected.h"
#include "wait.h"
#include "sslwrapper.h"
#include "sslcontext.h"

/**
 *  Set up namespace
 */
namespace AMQP { 

/**
 *  Class definition
 */
class SslHandshake : public TcpState, private Watchable
{
private:
    /**
     *  SSL structure
     *  @var SslWrapper
     */
    SslWrapper _ssl;

    /**
     *  The socket file descriptor
     *  @var int
     */
    int _socket;
    
    /**
     *  The outgoing buffer
     *  @var TcpOutBuffer
     */
    TcpOutBuffer _out;
    
    
    /**
     *  Report a new state
     *  @param  state
     *  @return TcpState
     */
    TcpState *nextstate(TcpState *state)
    {
        // forget the socket to prevent that it is closed by the destructor
        _socket = -1;
        
        // done
        return state;
    }
    
    /**
     *  Helper method to report an error
     *  @return TcpState*
     */
    TcpState *reportError()
    {
        // we are no longer interested in any events for this socket
        _handler->monitor(_connection, _socket, 0);
        
        // we have an error - report this to the user
        _handler->onError(_connection, "failed to setup ssl connection");
        
        // done, go to the closed state
        return new TcpClosed(_connection, _handler);
    }
    
    /**
     *  Proceed with the handshake
     *  @param  events      the events to wait for on the socket
     *  @return TcpState
     */
    TcpState *proceed(int events)
    {
        // tell the handler that we want to listen for certain events
        _handler->monitor(_connection, _socket, events);
        
        // allow chaining
        return this;
    }
    
public:
    /**
     *  Constructor
     * 
     *  @todo catch the exception!  
     * 
     *  @param  connection  Parent TCP connection object
     *  @param  socket      The socket filedescriptor
     *  @param  hostname    The hostname to connect to
     *  @param  context     SSL context
     *  @param  buffer      The buffer that was already built
     *  @param  handler     User-supplied handler object
     *  @throws std::runtime_error
     */
    SslHandshake(TcpConnection *connection, int socket, const std::string &hostname, TcpOutBuffer &&buffer, TcpHandler *handler) : 
        TcpState(connection, handler),
        _ssl(SslContext(SSLv23_client_method())),
        _socket(socket),
        _out(std::move(buffer))
    {       
        // we will be using the ssl context as a client
        SSL_set_connect_state(_ssl);
        
        // associate domain name with the connection
        SSL_set_tlsext_host_name(_ssl, hostname.data());
        
        // associate the ssl context with the socket filedescriptor
        if (SSL_set_fd(_ssl, socket) == 0) throw std::runtime_error("failed to associate filedescriptor with ssl socket");
        
        // we are going to wait until the socket becomes writable before we start the handshake
        _handler->monitor(_connection, _socket, writable);
    }
    
    /**
     *  Destructor
     */
    virtual ~SslHandshake() noexcept
    {
        // leap out if socket is invalidated
        if (_socket < 0) return;
        
        // close the socket
        close(_socket);
    }

    /**
     *  The filedescriptor of this connection
     *  @return int
     */
    virtual int fileno() const override { return _socket; }
    
    /**
     *  Process the filedescriptor in the object
     *  @param  fd          Filedescriptor that is active
     *  @param  flags       AMQP::readable and/or AMQP::writable
     *  @return             New state object
     */
    virtual TcpState *process(int fd, int flags) override
    {
        // must be the socket
        if (fd != _socket) return this;

        // start the ssl handshake
        int result = OpenSSL::SSL_do_handshake(_ssl);
                
        // if the connection succeeds, we can move to the ssl-connected state
        if (result == 1) return nextstate(new SslConnected(_connection, _socket, _ssl, std::move(_out), _handler));
        
        // error was returned, so we must investigate what is going on
        auto error = SSL_get_error(_ssl, result);
                            
        // check the error
        switch (error) {
        case SSL_ERROR_WANT_READ:   return proceed(readable);
        case SSL_ERROR_WANT_WRITE:  return proceed(readable | writable);
        default:                    return reportError();
        }
    }

    /**
     *  Send data over the connection
     *  @param  buffer      buffer to send
     *  @param  size        size of the buffer
     */
    virtual void send(const char *buffer, size_t size) override
    {
        // the handshake is still busy, outgoing data must be cached
        _out.add(buffer, size); 
    }
    
    /**
     *  Flush the connection, sent all buffered data to the socket
     *  @return TcpState    new tcp state
     */
    virtual TcpState *flush() override
    {
        // create an object to wait for the filedescriptor to becomes active
        Wait wait(_socket);
        
        // keep looping
        while (true)
        {
            // start the ssl handshake
            int result = SSL_do_handshake(_ssl);
        
            // if the connection succeeds, we can move to the ssl-connected state
            if (result == 1) return nextstate(new SslConnected(_connection, _socket, _ssl, std::move(_out), _handler));
        
            // error was returned, so we must investigate what is going on
            auto error = SSL_get_error(_ssl, result);
            
            // check the error
            switch (error) 
            {
                // if openssl reports that socket readability or writability is needed,
                // we wait for that until this situation is reached
                case SSL_ERROR_WANT_READ:   wait.readable(); break;
                case SSL_ERROR_WANT_WRITE:  wait.active(); break;
            
                // something is wrong, we proceed to the next state
                default: return reportError();
            }
        }
    }

    /**
     *  Report to the handler that the connection was nicely closed
     */
    virtual void reportClosed() override
    {
        // we no longer have to monitor the socket
        _handler->monitor(_connection, _socket, 0);

        // close the socket
        close(_socket);
        
        // socket is closed now
        _socket = -1;
        
        // copy the handler (if might destruct this object)
        auto *handler = _handler;
        
        // reset member before the handler can make a mess of it
        _handler = nullptr;
        
        // notify to handler
        handler->onClosed(_connection);
    }
};
    
/**
 *  End of namespace
 */
}

