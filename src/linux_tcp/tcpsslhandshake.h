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
#include "tcpsslconnected.h"
#include "wait.h"
#include "tcpssl.h"
#include "tcpsslcontext.h"

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
    TcpSsl _ssl;

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
    
public:
    /**
     *  Constructor
     * 
     * 	@todo catch the exception!	
     * 
     *  @param  connection  Parent TCP connection object
     *  @param  socket      The socket filedescriptor
     *  @param  hostname    The hostname to connect to
     *  @param  context     SSL context
     *  @param  buffer      The buffer that was already built
     *  @param  handler     User-supplied handler object
     * 	@throws std::runtime_error
     */
    TcpSslHandshake(TcpConnection *connection, int socket, const std::string &hostname, TcpOutBuffer &&buffer, TcpHandler *handler) : 
        TcpState(connection, handler),
        _ssl(TcpSslContext(SSLv23_client_method())),
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
    virtual ~TcpSslHandshake() noexcept
    {
        // close the socket
        // @todo only if really closed
        //close(_socket);
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
		if (result == 1) return new TcpSslConnected(_connection, _socket, _ssl, std::move(_out), _handler);
		
		// if there is a failure, we must close down the connection
		if (result <= 0) 
		{
			// error was returned, so we must investigate what is going on
			auto error = SSL_get_error(_ssl, result);
							
			// check the error
			switch (error) {
			case SSL_ERROR_WANT_READ:
				// the handshake must be repeated when socket is readable, wait for that
				_handler->monitor(_connection, _socket, readable);
				break;
			
			case SSL_ERROR_WANT_WRITE:
				// the handshake must be repeated when socket is readable, wait for that
				_handler->monitor(_connection, _socket, writable);
				break;
			
			case SSL_ERROR_WANT_ACCEPT:
				// the BIO was not connected yet, the SSL function should be called again
				_handler->monitor(_connection, _socket, writable);
				
				break;
			case  SSL_ERROR_WANT_X509_LOOKUP:
				_handler->monitor(_connection, _socket, writable);
				
				break;
			case SSL_ERROR_SYSCALL:
				_handler->monitor(_connection, _socket, writable);
				
				break;
			case  SSL_ERROR_SSL:
				_handler->monitor(_connection, _socket, writable);
				
				break;
			default:
				return reportError();
			}
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
			if (result == 1) return new TcpSslConnected(_connection, _socket, _ssl, std::move(_out), _handler);
		
			// error was returned, so we must investigate what is going on
			auto error = SSL_get_error(_ssl, result);
			
			// check the error
			switch (error) {
			case SSL_ERROR_WANT_READ:
				// wait for the socket to become readable
				if (!wait.readable()) return reportError();
				break;
			
			case SSL_ERROR_WANT_WRITE:
				// wait for the socket to become writable
				if (!wait.writable()) return reportError();
				break;

			default:
				// report an error
				return reportError();
			}
		}
		
        // keep same object (we never reach this code)
        return this;
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
