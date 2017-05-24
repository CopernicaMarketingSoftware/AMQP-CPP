/**
 *  Classes and includes for boost asio network operations
 * 
 *  @copyright 2014 TeamSpeak Systems GmbH
 */

#ifndef AMQP_CPP_BOOST_NETWORK_HANDLER_H
#define AMQP_CPP_BOOST_NETWORK_HANDLER_H

#include <string>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <mutex>
#include "boostreadbuffer.h"
#include "boostwritebuffer.h"
/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Template Class definition
 */
enum class ConnectionState{ CLOSED, CONNECTING, CONNECTED, CLOSING_WAIT_IO, CLOSING_WAIT_CALLBACK};

/**
 *  Pure virtual Interface Class definition for callbacks that BoostNetworkHandler makes
 */
class BoostNetworkHandlerCallbacks
 {
public:

	/**
     *  destructor
     */
	virtual ~BoostNetworkHandlerCallbacks(){}
	
	/**
     *  Method that is called when the connection succeeded
     */
	virtual void onConnectionOpened() = 0;
	
	/**
     *  Method that is called when the socket is closed (as a result of a BoostNetworkHandlerInterface::closeConnection() call)
     */
	virtual void onConnectionClosed() = 0;
	
	/**
     *  Method that is called when the socket connect or read operation times out
     */
	virtual void onConnectionTimeout(ConnectionState connectionState) = 0;
	
	/**
     *  Method that is called when the connection failed
     *  @param  connectionState state of the connection when the error occured
	 *  @param  ec error code
     */
	virtual void onIoError(ConnectionState connectionState, boost::system::error_code& ec) = 0;
	
	/**
     *  Method that is called when data is received on the socket
     *  @param  buffer      Pointer to the input buffer
	 *  @param bytesAvailable how many bytes on the buffer
	 *  @return bytes used
     */
	virtual std::size_t onIoBytesRead(char *buffer, std::size_t bytesAvailable)=0;
 };
 
 /**
 *  Pure virtual Interface Class definition for the public functions for BoostNetworkHandler
 */
 class BoostNetworkHandlerInterface
 {
public:

	/**
     *  opens the socket, contacting host at port
     *  @param  host      host name or ip
	 *  @param port
	 *  @return false if something went wrong immediately (connection was not in closed state), true otherwise
     */
	virtual bool openConnection(const std::string& host, uint16_t port) = 0;
	
	
	/**
     *  closes the connection
	 *  @param immediate if true, do not wait for pending write operations.
     */
	virtual void closeConnection(bool immediate) = 0;
	
	/**
     *  asynchroniously send the bytes
	 *  @param data pointer to bytes to send
	 *  @param size number of bytes to send
	 *  @return return false if something went wrong immediately (not in connected state?), true otherwise
     */
	virtual bool sendBytes(const char* data, std::size_t size) = 0;
	
	/**
     * set the callbacks to use by this handler
	 *  @param callbacks the object that implements the callbacks for this handler
     */
	virtual void set_callbacks(BoostNetworkHandlerCallbacks* callbacks) = 0;
 };
 
/**
 * class intended to do socket magic between connecting and reporting read to the callbacks.
 * This template is specialized in boostsslnetworkhandler.h to do the ssl handshake.
 * This generic implementation does nothing but call the completion handler
 */
template <typename SOCKET>
class PostConnectHandler{
public:
  template <typename CB>
  void afterConnect(SOCKET&, CB cb)
  {
	  boost::system::error_code ec;
	  cb(ec);
  }
};
 
 /**
  * The BoostNetworkHandler with abstract/generic socket type
  */
 template <typename SOCKET>
 class BoostNetworkHandler : public BoostNetworkHandlerInterface
 {
private:
	typedef boost::asio::ip::tcp::resolver ResolverType;
	typedef typename ResolverType::query QueryType;

	 /**
     *  The SOCKET
     *  @var    SOCKET
     */
	SOCKET& _socket;
	
	 /**
     *  A timer used to detect stale reads / connects
     *  @var    boost::asio::deadline_timer
     */	
	boost::asio::deadline_timer _readTimer;
	
	 /**
     *  A  buffer for all pending writes
     *  @var    BoostWriteBuffer
     */	
	BoostWriteBuffer _writeBuffer;

	 /**
     *  A  buffer for all unconsumed read bytes
     *  @var    BoostReadBuffer
     */	
	BoostReadBuffer _readBuffer;
	
	 /**
     *  A  mutex to protect _connectionState, _reading, _writing 
     *  @var    std::mutex
     */	
	std::mutex _mutex;

	 /**
     *  current state of the connection
     *  @var    ConnectionState
     */	
	ConnectionState _connectionState;
	
	 /**
     *  does the object have an outstanding read
     *  @var    bool
     */	
	bool _reading;
	
	 /**
     *  does the object have an outstanding write
     *  @var    bool
     */	
	bool _writing;
	
	 /**
     *  HearBeat seconds of the amqp sever. If nothing is received from amqp after this many seconds,
	 * the connection is assumed to be broken
     *  @var    unsigned
     */	
	unsigned _heartBeatSeconds;
	
	 /**
     * The dns resolved object
     *  @var    ResolverType
     */	
	ResolverType _resolver;

	 /**
     * pointer to the callbacks
     *  @var    BoostNetworkHandlerCallbacks
     */	
	BoostNetworkHandlerCallbacks* _callbacks;
	
	 /**
     * a helper class for doing operations between connect and reporting ready
     *  @var    PostConnectHandler<SOCKET>
     */	
	PostConnectHandler<SOCKET> _postConnectHandler;
	public:
	
	/**
	*  Constructor
	* @param socket the socket
	* @param hearBeatSeconds: the hearbeat configured by the amqp server
	*/
	BoostNetworkHandler(SOCKET& socket, unsigned heartBeatSeconds = 580)
	: _socket(socket)
	, _readTimer(socket.get_io_service())
	, _writeBuffer()
	, _readBuffer(8192, 16384)
	, _mutex()
	, _connectionState(ConnectionState::CLOSED)
	, _reading(false)
	, _writing(false)
	, _heartBeatSeconds(heartBeatSeconds)
	, _resolver(socket.get_io_service())
	, _callbacks()
	, _postConnectHandler()
	{}
	
	/**
     *  opens the socket, contacting host at port
     *  @param  host      host name or ip
	 *  @param port
	 *  @return false if something went wrong immediately (connection was not in closed state), true otherwise
     */	
	bool openConnection(const std::string& host, uint16_t port) override
	{
		{
			std::lock_guard<std::mutex> lock(_mutex);
			if (_connectionState != ConnectionState::CLOSED) return false;
			_connectionState = ConnectionState::CONNECTING;
		}
		
		QueryType query(host, std::to_string(port),  QueryType::address_configured | QueryType::numeric_service);
		_resolver.async_resolve(query, std::bind(&BoostNetworkHandler::onResolved, this, std::placeholders::_1, std::placeholders::_2));
		return true;
	}
	
	/**
     *  closes the connection
	 *  @param immediate if true, do not wait for pending write operations.
     */
	void closeConnection(bool immediate) override
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_connectionState != ConnectionState::CONNECTING && _connectionState != ConnectionState::CONNECTED) return;
		
		_connectionState = ConnectionState::CLOSING_WAIT_IO;
		if (!_writing || immediate ) doClose();
		if (!_writing && !_reading) doCloseCallback();
		
		boost::system::error_code ec;
		_readTimer.cancel(ec);
	}
	
	/**
     *  asynchroniously send the bytes
	 *  @param data pointer to bytes to send
	 *  @param size number of bytes to send
	 *  @return return false if something went wrong immediately (not in connected state?), true otherwise
     */
	bool sendBytes(const char* data, std::size_t size) override
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_connectionState != ConnectionState::CONNECTED) return false;
		
		//make a copy of the data and store it in a list of to-send data
		_writeBuffer.append(data,size);
		
		//if we are not currently writing, we need to start a new write operation
		if (!_writing)
		{
			//now we are writing
			_writing=true;
			
			//create a new temporary BoostWriteBuffer
			BoostWriteBuffer* temp = new BoostWriteBuffer();
			
			//swap contents with _writebuffer. _writebuffer is now empty while temp holds all the data
			temp->swap(_writeBuffer);
			
			//send this temoprary buffer
			boost::asio::async_write(_socket, BoostWriteBuffer::BufferAdapter(*temp), std::bind(&BoostNetworkHandler::onBytesWritten, this, std::placeholders::_1, temp ));
		}
		return true;
	}
	
	/**
     * set the callbacks to use by this handler
	 *  @param callbacks the object that implements the callbacks for this handler
     */
	void set_callbacks(BoostNetworkHandlerCallbacks* callbacks) override
	{
		_callbacks = callbacks;
	}

private:
	void onConnectionOpened()
	{
		_callbacks->onConnectionOpened();
	};
	
	void onConnectionClosed()
	{
		_callbacks->onConnectionClosed();
	}
	
	void onIoError(ConnectionState connectionState, boost::system::error_code& ec)
	{
		_callbacks->onIoError(connectionState, ec);
	}
	
	std::size_t onIoBytesRead(char *buffer, std::size_t bytesAvailable)
	{
		return _callbacks->onIoBytesRead(buffer, bytesAvailable);
	}
	
	void onResolved(const boost::system::error_code& error, typename ResolverType::iterator iterator)
	{
		if (error)
		{
			_socket.get_io_service().post(std::bind(&BoostNetworkHandler::onIoError, this, _connectionState, error));
			_connectionState = ConnectionState::CLOSED;
			return;
		}
		
		boost::system::error_code ec;
		_readTimer.expires_from_now(boost::posix_time::seconds(10), ec);
		if (ec)
		{
			_socket.get_io_service().post(std::bind(&BoostNetworkHandler::onIoError, this, _connectionState, ec));
			_connectionState = ConnectionState::CLOSED;
			return;
		}
		_readTimer.async_wait(std::bind(&BoostNetworkHandler::onTimerExpired, this, std::placeholders::_1, _connectionState) );
		
		_socket.lowest_layer().async_connect(*iterator, std::bind(&BoostNetworkHandler::onSocketConnected, this, std::placeholders::_1));
	}
	
	void onSocketConnected(const boost::system::error_code& error)
	{
		if (error)
		{
			std::lock_guard<std::mutex> lock(_mutex);
			_socket.get_io_service().post(std::bind(&BoostNetworkHandler::onIoError, this, _connectionState, error));
			_connectionState = ConnectionState::CLOSED;
		}
		else
		{
			_postConnectHandler.afterConnect(_socket, std::bind(&BoostNetworkHandler::onSocketReady, this, std::placeholders::_1));
		}
	}

	void onSocketReady(const boost::system::error_code& error)
	{
		std::lock_guard<std::mutex> lock(_mutex);

		//cancel current read timer, if any
		boost::system::error_code ec;
		_readTimer.cancel(ec);

		if (error) 
		{
			boost::system::error_code ec;
			_socket.lowest_layer().close(ec);
			
			_socket.get_io_service().post(std::bind(&BoostNetworkHandler::onIoError, this, _connectionState, error));
			_connectionState = ConnectionState::CLOSED;
		}
		else
		{
			_reading = true;
			_connectionState = ConnectionState::CONNECTED;
			_socket.get_io_service().post(std::bind(&BoostNetworkHandler::onConnectionOpened, this));
			doStartNewRead();
		}
	}

	void onBytesRead(const boost::system::error_code& error, std::size_t bytesTransferred)
	{
		//cancel current read timer, if any
		boost::system::error_code ec;
		_readTimer.cancel(ec);
		
		if (error) 
		{
			bytesTransferred = 0;	
		}
		else
		{
			if (bytesTransferred==0) 
			{
				//other end closed the connetion. Report this.
				_socket.get_io_service().post(std::bind(&BoostNetworkHandler::onIoError, this, _connectionState, error));
				return;
			}
		}
		_readBuffer.commit(bytesTransferred);
		
		if (error)
		{
			std::lock_guard<std::mutex> lock(_mutex);
			_reading = false;
			if (_connectionState == ConnectionState::CLOSING_WAIT_CALLBACK)
			{
				if (!_writing) doCloseCallback();
			}
			else
			{
				_socket.get_io_service().post(std::bind(&BoostNetworkHandler::onIoError, this, _connectionState, error));
			}
		}
		else
		{
			std::size_t bytesUsed = onIoBytesRead(_readBuffer.data(), _readBuffer.size());
			_readBuffer.remove(bytesUsed);
			
			{
				std::lock_guard<std::mutex> lock(_mutex);
				if (_connectionState == ConnectionState::CLOSING_WAIT_IO || _connectionState == ConnectionState::CLOSING_WAIT_CALLBACK)
				{
					_reading = false;
					if (_connectionState == ConnectionState::CLOSING_WAIT_CALLBACK && !_writing) doCloseCallback();
				}
				else
				{
					doStartNewRead();
				}
			}
		}
	}
	
	void onBytesWritten(const boost::system::error_code& error, BoostWriteBuffer* buf)
	{
		//first delete the write buffer we were sending
		delete buf;
		
		//handle errors if needed, or start a new write if needed
		std::lock_guard<std::mutex> lock(_mutex);
		if (error)
		{
			_writing = false;
			if (_connectionState == ConnectionState::CLOSING_WAIT_CALLBACK && !_reading) 
			{
				doCloseCallback();
			}
			else
			{
				_socket.get_io_service().post(std::bind(&BoostNetworkHandler::onIoError, this, _connectionState, error));
			}
		}
		else
		{
			if (_writeBuffer.hasData())
			{
				BoostWriteBuffer* temp = new BoostWriteBuffer();
				temp->swap(_writeBuffer);
				boost::asio::async_write(_socket, BoostWriteBuffer::BufferAdapter(*temp), std::bind(&BoostNetworkHandler::onBytesWritten, this, std::placeholders::_1, temp ));
			}
			else
			{
				_writing = false;
				if (_connectionState == ConnectionState::CLOSING_WAIT_IO) doClose();
				if (_connectionState == ConnectionState::CLOSING_WAIT_CALLBACK && !_reading) doCloseCallback();
			}
		}
	}
	
	void onTimerExpired(const boost::system::error_code& error, ConnectionState connectionState)
	{
		if (error == boost::asio::error::operation_aborted) return;
		_socket.get_io_service().post(std::bind(&BoostNetworkHandlerCallbacks::onConnectionTimeout, _callbacks, connectionState));
	}
	
	void doClose()
	{
		 boost::system::error_code ec;
		_socket.lowest_layer().close(ec);
		_connectionState = ConnectionState::CLOSING_WAIT_CALLBACK;
	}
	
	void doCloseCallback()
	{
		_socket.get_io_service().post(std::bind(&BoostNetworkHandler::onConnectionClosed, this));
		_connectionState = ConnectionState::CLOSED;
	}

	void doStartNewRead()
	{
		boost::system::error_code ec;
		_readTimer.expires_from_now(boost::posix_time::seconds(_heartBeatSeconds), ec);
		if (ec)
		{
			_socket.get_io_service().post(std::bind(&BoostNetworkHandler::onIoError, this, _connectionState, ec));
			_connectionState = ConnectionState::CLOSED;
			return;
		}
		_readTimer.async_wait(std::bind(&BoostNetworkHandler::onTimerExpired, this, std::placeholders::_1, _connectionState) );
		
		char* data = _readBuffer.prepare();
		std::size_t size = _readBuffer.readChunkSize();
		_socket.async_read_some(boost::asio::buffer(data, size), std::bind(&BoostNetworkHandler::onBytesRead, this, std::placeholders::_1, std::placeholders::_2));
	}
};

/**
 * a small helper function to convert ConnectionState to string
 * @param cs the connectionstate
 * @return the string representation of cs
 */
inline
std::string connectionStateToString(ConnectionState cs)
{
	switch (cs)
	{
		case ConnectionState::CLOSED: return "CLOSED";
		case ConnectionState::CONNECTING: return "CONNECTING";
		case ConnectionState::CONNECTED: return "CONNECTED";
		case ConnectionState::CLOSING_WAIT_IO: return "CLOSING_IO_WAIT";
		case ConnectionState::CLOSING_WAIT_CALLBACK: return "CLOSING_WAIT_CALLBACK";
	}
	return "INVALID CONNECTION STATE VALUE";
}

/**
* end namespace
*/
}
#endif //AMQP_CPP_BOOST_NETWORK_HANDLER_H