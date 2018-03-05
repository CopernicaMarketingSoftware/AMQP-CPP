/**
 * TcpSslConnected.h
 *
 * The actual tcp connection over SSL
 *
 * @copyright 2018 copernica BV
 */
 
/** 
  * Include guard
  */
#pragma once

/**
 * Dependencies
 */
#include "tcpoutbuffer.h"
#include "tcpinbuffer.h"
#include "wait.h"
#include <openssl/ssl.h>

#include <iostream>

/**
 * Set up namespace
 */
namespace AMQP {

/** 
 * Class definition
 */	
class TcpSslConnected: public TcpState, private Watchable 
{
private:
	/**
	 *  The SSL context
	 *  @var SSL*
	 */
	SSL *_ssl;

	/** 
	 * 	Socket file descriptor
	 * 	@var int
	 */
	int _socket;
	 
	/**
	 *  The outgoing buffer
	 *  @var TcpBuffer
	 */
	TcpOutBuffer _out;

	/**
	 *  The incoming buffer
	 *  @var TcpInBuffer
	 */
	TcpInBuffer _in;
	
	/**
	 *  Are we now busy with sending or receiving?
	 * 	@var enum
	 */
	enum {
		state_idle,
		state_sending,
		state_receiving
	} _state;
	

	/**
     *  Helper method to report an error
     *  @return bool        Was an error reported?
     */
    bool reportError()
    {
        // some errors are ok and do not (necessarily) mean that we're disconnected
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) return false;
        
        // we have an error - report this to the user
        _handler->onError(_connection, strerror(errno));
        
        // done
        return true;
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
	 *  Proceed with the previous operation, possibly changing the monitor
	 * 	@return TcpState*
	 */
	TcpState *proceed()
	{
		// if we still have an outgoing buffer we want to send out data
		if (_out)
		{
			// we still have a buffer with outgoing data
			_state = state_sending;
		
			// let's wait until the socket becomes writable
			_handler->monitor(_connection, _socket, readable);
		}
		else
		{
			// outgoing buffer is empty, we're idle again waiting for further input
			_state = state_idle;
			
			// let's wait until the socket becomes readable
			_handler->monitor(_connection, _socket, readable);
		}
		
		// done
		return this;
	}
	
	/**
	 *  Method to repeat the previous call
	 *  @param	result		result of an earlier openssl operation
	 * 	@return TcpState*
	 */
	TcpState *repeat(int result)
	{
		// error was returned, so we must investigate what is going on
		auto error = SSL_get_error(_ssl, result);
						
		std::cout << "error = " << error << std::endl;
						
		// check the error
		switch (error) {
		case SSL_ERROR_WANT_READ:
			// the operation must be repeated when readable
			std::cout << "want read" << std::endl;
			
			_handler->monitor(_connection, _socket, readable);
			return this;
		
		case SSL_ERROR_WANT_WRITE:
			// wait until socket becomes writable again
			std::cout << "want write" << std::endl;

			_handler->monitor(_connection, _socket, writable);
			return this;
			
		default:
			std::cout << "something else" << std::endl;

			// @todo check how to handle this
			return this;
		}
	}
	
	
public:
	/**
	 * Constructor
	 * @param	connection 	Parent TCP connection object
	 * @param	socket		The socket filedescriptor
	 * @param	ssl			The SSL structure
	 * @param	buffer		The buffer that was already built
	 * @param	handler		User-supplied handler object
	 */
	TcpSslConnected(TcpConnection *connection, int socket, SSL *ssl, TcpOutBuffer &&buffer, TcpHandler *handler) : 
		TcpState(connection, handler),
		_ssl(ssl),
        _socket(socket),
        _out(std::move(buffer)),
        _in(4096),
        _state(_out ? state_sending : state_idle)
	{
		std::cout << "ssl-connected" << std::endl;
		
		// tell the handler to monitor the socket if there is an out
		_handler->monitor(_connection, _socket, _state == state_sending ? writable : readable); 
	}	
	
	/**
	 * Destructor
	 */
	virtual ~TcpSslConnected() noexcept
	{
		// skip if handler is already forgotten
		if (_handler == nullptr) return;
		
		// we no longer have to monitor the socket
		_handler->monitor(_connection, _socket, 0);
		
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
     *  @param  fd          The filedescriptor that is active
     *  @param  flags       AMQP::readable and/or AMQP::writable
     *  @return             New implementation object
     */
    virtual TcpState *process(int fd, int flags)
    {
		std::cout << "process call in ssl-connected" << std::endl;
		
		std::cout << fd << " - " << _socket << std::endl;
		
		
		// the socket must be the one this connection writes to
		if (fd != _socket) return this;
		
        // because the object might soon be destructed, we create a monitor to check this
        Monitor monitor(this);

		// are we busy with sending or receiving data?
		if (_state == state_sending)
        {
			std::cout << "busy sending" << std::endl;
			
			// try to send more data from the outgoing buffer
			auto result = _out.sendto(_ssl);
			
			std::cout << "result = " << result << std::endl;
			
			// if this is a success, we may have to update the monitor
			if (result > 0) return proceed();
			
			// the operation failed, we may have to repeat our call
			else return repeat(result);
		}
		else
		{
            // read data from ssl into the buffer
            auto result = _in.receivefrom(_ssl, _connection->expected());

			// if this is a success, we may have to update the monitor
			// @todo also parse the buffer
			if (result > 0) return proceed();
			
			// the operation failed, we may have to repeat our call
			else return repeat(result);



			// we're busy with receiving data
			// @todo check this
			
			std::cout << "receive data" << std::endl;
			
        }
        
        // keep same object
		return this;
    }

    /**
     *  Send data over the connection
     *  @param  buffer      buffer to send
     *  @param  size        size of the buffer
     */
    virtual void send(const char *buffer, size_t size)
    {
		// put the data in the outgoing buffer
		_out.add(buffer, size);
		
		// if we're already busy with sending or receiving, we first have to wait
		// for that operation to complete before we can move on
		if (_state != state_idle) return;
		
		// let's wait until the socket becomes writable
		_handler->monitor(_connection, _socket, writable);
    }
}; 

/**
 *  End of namespace
 */
}
 
