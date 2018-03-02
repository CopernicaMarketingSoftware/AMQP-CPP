/**
 *	TcpSslHandshake.h
 *
 *	Implementation of the TCP state that is responsible for setting
 *	up the STARTTLS handshake.
 *
 *	@copyright 2018 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include "tcpoutbuffer.h"

#include <openssl/ssl.h>
#include <copernica/dynamic.h>

#include <iostream>

/**
 *  Set up namespace
 */
namespace AMQP { 

/**
 *  Class definition
 */
class TcpSslHandshake : public TcpState, private Watchable
{
private:

    /**
     *  SSL structure
     *  @var SSL
     */
    SSL *_ssl;

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
     *  Helper method to report an error
     *  @return TcpState*
     */
    TcpState *reportError()
    {
		// we are no longer interested in any events for this socket
		_handler->monitor(_connection, _socket, 0);
		
        // we have an error - report this to the user
        _handler->onError(_connection, "failed to setup ssl connection");
        
        // close the socket
        close(_socket);
        
        // done, go to the closed state
        return new TcpClosed(_connection, _handler);
    }
    
    /**
     *  Construct the next state
     *  @param  monitor     Object that monitors whether connection still exists
     *  @return TcpState*
     */
    TcpState *nextState(const Monitor &monitor)
    {
        // if the object is still in a valid state, we can move to the close-state, 
        // otherwise there is no point in moving to a next state
        return monitor.valid() ? new TcpClosed(this) : nullptr;
    }
    
    /**
     *  Wait until the socket is writable
     *  @return bool
     */
    bool wait4writable()
    {
        // we need the fd-sets
        fd_set readables, writables, exceptions;
        
        // initialize all the sets
        FD_ZERO(&readables);
        FD_ZERO(&writables);
        FD_ZERO(&exceptions);
        
        // add the one socket
        FD_SET(_socket, &writables);
        
        // wait for the socket
        auto result = select(_socket + 1, &readables, &writables, &exceptions, nullptr);
        
        // check for success
        return result == 0;
    }
    
public:
    /**
     *  Constructor
     * 
     * 	@todo catch the exception!	
     * 
     *  @param  connection  Parent TCP connection object
     *  @param  socket      The socket filedescriptor
     *  @param  context     SSL context
     *  @param  buffer      The buffer that was already built
     *  @param  handler     User-supplied handler object
     * 	@throws std::runtime_error
     */
    TcpSslHandshake(TcpConnection *connection, int socket, TcpOutBuffer &&buffer, TcpHandler *handler) : 
        TcpState(connection, handler),
        _socket(socket),
        _out(std::move(buffer))
    {
		SSL_library_init();
		
		// create ssl object
		_ssl = SSL_new(SSL_CTX_new(TLS_client_method()));

		// leap out on error
		if (_ssl == nullptr) throw std::runtime_error("ERROR: SSL structure is null");

		// we will be using the ssl context as a client
		// @todo check return value
		SSL_set_connect_state(_ssl);
		
		// associate the ssl context with the socket filedescriptor
		// @todo check return value
		SSL_set_fd(_ssl, socket);
		
		// we are going to wait until the socket becomes writable before we start the handshake
        _handler->monitor(_connection, _socket, writable);
    }
    
    /**
     *  Destructor
     */
    virtual ~TcpSslHandshake() noexcept
    {
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
		int result = SSL_do_handshake(_ssl);
		
		// if the connection succeeds, we can move to the ssl-connected state
		// @todo we need the sslconnected state object
		if (result == 1) return this; // new TcpSslConnected(connection, socket, _ssl, std::move(_out), _handler);
		
		// if there is a failure, we must close down the connection
		if (result == 0) return reportError();
		
		// -1 was returned, so we must investigate what is going on
		auto error = SSL_get_error(_ssl, result);
			
		// check the error
		switch (error) {
		case SSL_ERROR_WANT_READ:
			// the handshake must be repeated when socket is readable, wait for that
			std::cout << "wait for readability" << std::endl;
			_handler->monitor(_connection, _socket, readable);
			break;
		
		case SSL_ERROR_WANT_WRITE:
			// the handshake must be repeated when socket is readable, wait for that
			std::cout << "wait for writability" << std::endl;
			_handler->monitor(_connection, _socket, writable);
			break;
		
		default:
			// @todo implement handling other error states
			std::cout << "unknown error state " << error << std::endl;
			// @todo we have to close the connection
			return reportError();
		}
	
        // keep same object
        return this;
    }

    /**
     *  Send data over the connection
     *  @param  buffer      buffer to send
     *  @param  size        size of the buffer
     */
    virtual void send(const char *buffer, size_t size) override
    {
		
		// @todo because the handshake is still busy, outgoing data must be cached
		
    }
    
    /**
     *  Flush the connection, sent all buffered data to the socket
     *  @return TcpState    new tcp state
     */
    virtual TcpState *flush() override
    {
		// @todo implementation?
		return nullptr;
    }

    /**
     *  Report that heartbeat negotiation is going on
     *  @param  heartbeat   suggested heartbeat
     *  @return uint16_t    accepted heartbeat
     */
    virtual uint16_t reportNegotiate(uint16_t heartbeat) override
    {
		/*
		 *  @todo what should we do here?
		
        // remember that we have to reallocated (_in member can not be accessed because it is moved away)
        _reallocate = _connection->maxFrame();
        
        // pass to base
        return TcpState::reportNegotiate(heartbeat);
        */
        
        return 0;
    }

    /**
     *  Report to the handler that the connection was nicely closed
     */
    virtual void reportClosed() override
    {
		/*
		
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
        */
    }
};
    
/**
 *  End of namespace
 */
}
