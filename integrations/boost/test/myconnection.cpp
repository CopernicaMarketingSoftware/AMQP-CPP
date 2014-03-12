/**
 *  MyConnection.cpp
 *
 *  @copyright 2014 Copernica BV
 *  @copyright 2014 TeamSpeak Systems GmbH
 */
 
/**
 *  Required external libraries
 */
#include <amqpcpp.h>
#include <string>

/**
 *  Namespaces to use
 */
using namespace std;

/**
 *  Required local class definitions
 */
#include "myconnection.h"

/**
 *  Constructor
 */
MyConnection::MyConnection(AMQP::BoostNetworkHandlerInterface* networkHandler, const std::string &ip, std::uint16_t port) :
    _networkHandler(networkHandler),
    _connection(nullptr),
    _channel(nullptr)
{
	_networkHandler->set_callbacks(this);
	
    // start connecting
    if (!_networkHandler->openConnection(ip, port))
	{
	  //something went wrong
	  return;
	}
}

/**
 *  Destructor
 */
MyConnection::~MyConnection()
{
    // do we still have a channel?
    if (_channel)  delete _channel;
    
    // do we still have a connection?
    if (_connection) delete _connection;
}

/**
 *  Method that is called when the connection failed
 *  @param  connectionState state of the connection when the error occured
 *  @param  ec error code
*/
void MyConnection::onIoError(AMQP::ConnectionState connectionState, boost::system::error_code& ec) 
{
    // report error
	if (!ec) 
	{
		std::cout << "connection was terminated by server" << std::endl;
		return;
	}
    
	std::cout << "io failure during state: "<< AMQP::connectionStateToString(connectionState) <<
		" cat: "<< ec.category().name()<<" error msg: " << ec.message() << std::endl;
}

/**
 *  Method that is called when the connection succeeded
 */
void MyConnection::onConnectionOpened()
{
    // report connection
    std::cout << "connected" << std::endl;
    
    // we are connected, leap out if there already is a amqp connection
    if (_connection) return;
    
    // create amqp connection, and a new channel
    _connection = new AMQP::Connection(this, AMQP::Login("guest", "guest"), "/");
    _channel = new AMQP::Channel(_connection, this);
    
    // we declare a queue, an exchange and we publish a message
    _channel->declareQueue("my_queue");
//    _channel->declareQueue("my_queue", AMQP::autodelete);
    _channel->declareExchange("my_exchange", AMQP::direct);
    _channel->bindQueue("my_exchange", "my_queue", "key");
}

/**
 *  Method that is called when the socket is closed (as a result of a BoostNetworkHandlerInterface::closeConnection() call)
 */
void MyConnection::onConnectionClosed()
{
    // show
    std::cout << "myconnection closed" << std::endl;

    // close the channel and connection
    if (_channel) delete _channel;
    if (_connection) delete _connection;
    
    // set to null
    _channel = nullptr;
    _connection = nullptr;
}

/**
 *  Method that is called when the socket connect or read operation times out
 */
void MyConnection::onConnectionTimeout(AMQP::ConnectionState connectionState)
{
	std::cout << "read/connect timed out" << std::endl;
	_networkHandler->closeConnection(true);
}

/**
 *  Method that is called when data is received on the socket
 *  @param  buffer      Pointer to the fill input buffer
 *  @param bytesAvailable how many bytes on the buffer
 *  @return bytes used
 */
std::size_t MyConnection::onIoBytesRead(char *buffer, std::size_t bytesAvailable)
{
    // leap out if there is no connection
    if (!_connection) return 0;

    // let the data be handled by the connection
    std::size_t consumed = _connection->parse(buffer, bytesAvailable);

	// send what came in
    std::cout << "received: " << bytesAvailable << " bytes. consumed: " << consumed << std::endl;
    
    return consumed;
}

/**
 *  Method that is called when data needs to be sent over the network
 *
 *  Note that the AMQP library does no buffering by itself. This means 
 *  that this method should always send out all data or do the buffering
 *  itself.
 *
 *  @param  connection      The connection that created this output
 *  @param  buffer          Data to send
 *  @param  size            Size of the buffer
 */
void MyConnection::onData(AMQP::Connection *connection, const char *buffer, size_t size)
{
    // send to the socket
    if (!_networkHandler->sendBytes(buffer, size))
	{
		std::cout << "error sending bytes to network" << std::endl;
	}
}

/**
 *  Method that is called when the connection was closed.
 * 
 *  This is the counter part of a call to Connection::close() and it confirms
 *  that the connection was correctly closed.
 * 
 *  @param  connection      The connection that was closed and that is now unusable
 */
void MyConnection::onClosed(AMQP::Connection *connection)
{
	 std::cout << "AMQP closed" << std::endl;
	 if (_connection)
	 { 
		 std::cout << "closing network connection " << std::endl;
		_networkHandler->closeConnection(false);
	 }
}

/**
 *  When the connection ends up in an error state this method is called.
 *  This happens when data comes in that does not match the AMQP protocol
 *  
 *  After this method is called, the connection no longer is in a valid
 *  state and can be used. In normal circumstances this method is not called.
 *
 *  @param  connection      The connection that entered the error state
 *  @param  message         Error message
 */
void MyConnection::onError(AMQP::Connection *connection, const std::string &message)
{
    // report error
    std::cout << "AMQP Connection error: " << message << std::endl;
}

/**
 *  Method that is called when the login attempt succeeded. After this method
 *  was called, the connection is ready to use
 *
 *  @param  connection      The connection that can now be used
 */
void MyConnection::onConnected(AMQP::Connection *connection)
{
    // show
    std::cout << "AMQP login success" << std::endl;
    
    // create channel if it does not yet exist
    if (!_channel) _channel = new AMQP::Channel(connection, this);
}

/**
 *  Method that is called when the channel was succesfully created.
 *  Only after the channel was created, you can use it for subsequent messages over it
 *  @param  channel
 */
void MyConnection::onReady(AMQP::Channel *channel)
{
    // show
    std::cout << "AMQP channel ready, id: " << (int) channel->id() << std::endl;
}

/**
 *  An error has occured on the channel
 *  @param  channel
 *  @param  message
 */

void MyConnection::onError(AMQP::Channel *channel, const std::string &message)
{
    // show
    std::cout << "AMQP channel error, id: " << (int) channel->id() << " - message: " << message << std::endl;

    // main channel cause an error, get rid of if
    delete _channel;

    // reset pointer
    _channel = nullptr;
}
    
/**
 *  Method that is called when the channel was paused
 *  @param  channel
 */
void MyConnection::onPaused(AMQP::Channel *channel)
{
    // show
    std::cout << "AMQP channel paused" << std::endl;
}

/**
 *  Method that is called when the channel was resumed
 *  @param  channel
 */
void MyConnection::onResumed(AMQP::Channel *channel)
{
    // show
    std::cout << "AMQP channel resumed" << std::endl;
}

/**
 *  Method that is called when a channel is closed
 *  @param  channel
 */
void MyConnection::onClosed(AMQP::Channel *channel)
{
    // show
    std::cout << "AMQP channel closed" << std::endl;
}

/**
 *  Method that is called when a transaction was started
 *  @param  channel
 */
void MyConnection::onTransactionStarted(AMQP::Channel *channel)
{
    // show
    std::cout << "AMQP transaction started" << std::endl;
}

/**
 *  Method that is called when a transaction was committed
 *  @param  channel
 */
void MyConnection::onTransactionCommitted(AMQP::Channel *channel)
{
    // show
    std::cout << "AMQP transaction committed" << std::endl;
}

/**
 *  Method that is called when a transaction was rolled back
 *  @param  channel
 */
void MyConnection::onTransactionRolledBack(AMQP::Channel *channel)
{
    // show
    std::cout << "AMQP transaction rolled back" << std::endl;
}

/**
 *  Mehod that is called when an exchange is declared
 *  @param  channel
 */
void MyConnection::onExchangeDeclared(AMQP::Channel *channel)
{
    // show
    std::cout << "AMQP exchange declared" << std::endl;
}

/**
 *  Method that is called when an exchange is bound
 *  @param  channel
 */
void MyConnection::onExchangeBound(AMQP::Channel *channel)
{
    // show
    std::cout << "AMQP Exchange bound" << std::endl;
}

/**
 *  Method that is called when an exchange is unbound
 *  @param  channel
 */
void MyConnection::onExchangeUnbound(AMQP::Channel *channel)
{
    // show
    std::cout << "AMQP Exchange unbound" << std::endl;
}

/**
 *  Method that is called when an exchange is deleted
 *  @param  channel
 */
void MyConnection::onExchangeDeleted(AMQP::Channel *channel)
{
    // show
    std::cout << "AMQP Exchange deleted" << std::endl;
}

/**
 *  Method that is called when a queue is declared
 *  @param  channel
 *  @param  name            name of the queue
 *  @param  messageCount    number of messages in queue
 *  @param  consumerCount   number of active consumers
 */
void MyConnection::onQueueDeclared(AMQP::Channel *channel, const std::string &name, uint32_t messageCount, uint32_t consumerCount)
{
    // show
    std::cout << "AMQP Queue declared" << std::endl;
}

/**
 *  Method that is called when a queue is bound
 *  @param  channel
 *  @param  
 */
void MyConnection::onQueueBound(AMQP::Channel *channel)
{
    // show
    std::cout << "AMQP Queue bound" << std::endl;

//    _connection->setQos(10);
//    _channel->setQos(1);


//    _channel->publish("my_exchange", "invalid-key", AMQP::mandatory, "this is the message");
    _channel->publish("my_exchange", "key", AMQP::mandatory, "this is the message");
    _channel->consume("my_queue");
}

/**
 *  Method that is called when a queue is deleted
 *  @param  channel
 *  @param  messageCount    number of messages deleted along with the queue
 */
void MyConnection::onQueueDeleted(AMQP::Channel *channel, uint32_t messageCount)
{
    // show
    std::cout << "AMQP Queue deleted" << std::endl;
}

/**
 *  Method that is called when a queue is unbound
 *  @param  channel
 */
void MyConnection::onQueueUnbound(AMQP::Channel *channel)
{
    // show
    std::cout << "AMQP Queue unbound" << std::endl;
}

/**
 *  Method that is called when a queue is purged
 *  @param  messageCount        number of message purged
 */
void MyConnection::onQueuePurged(AMQP::Channel *channel, uint32_t messageCount)
{
    // show
    std::cout << "AMQP Queue purged" << std::endl;
}

/**
 *  Method that is called when the quality-of-service was changed
 *  This is the result of a call to Channel::setQos()
 */
void MyConnection::onQosSet(AMQP::Channel *channel)
{
    // show
    std::cout << "AMQP Qos set" << std::endl;
}

/**
 *  Method that is called when a consumer was started
 *  This is the result of a call to Channel::consume()
 *  @param  channel         the channel on which the consumer was started
 *  @param  tag             the consumer tag
 */
void MyConnection::onConsumerStarted(AMQP::Channel *channel, const std::string &tag)
{
    // show
    std::cout << "AMQP consumer started" << std::endl;
}

/**
 *  Method that is called when a message has been received on a channel
 *  @param  channel         the channel on which the consumer was started
 *  @param  message         the consumed message
 *  @param  deliveryTag     the delivery tag, you need this to acknowledge the message
 *  @param  consumerTag     the consumer identifier that was used to retrieve this message
 *  @param  redelivered     is this a redelivered message?
 */
void MyConnection::onReceived(AMQP::Channel *channel, const AMQP::Message &message, uint64_t deliveryTag, const std::string &consumerTag, bool redelivered)
{
    // show
    std::cout << "AMQP consumed: " << message.message() << std::endl;
    
    // ack the message
    channel->ack(deliveryTag);
	
	std::cout << "recieved one message: closing connection now "<< std::endl;
	if (_connection) _connection->close();
}

/**
 *  Method that is called when a message you tried to publish was returned
 *  by the server. This only happens when the 'mandatory' or 'immediate' flag
 *  was set with the Channel::publish() call.
 *  @param  channel         the channel on which the message was returned
 *  @param  message         the returned message
 *  @param  code            the reply code
 *  @param  text            human readable reply reason
 */
void MyConnection::onReturned(AMQP::Channel *channel, const AMQP::Message &message, int16_t code, const std::string &text)
{
    // show
    std::cout << "AMQP message returned: " << text << std::endl;
}

/**
 *  Method that is called when a consumer was stopped
 *  This is the result of a call to Channel::cancel()
 *  @param  channel         the channel on which the consumer was stopped
 *  @param  tag             the consumer tag
 */
void MyConnection::onConsumerStopped(AMQP::Channel *channel, const std::string &tag)
{
    // show
    std::cout << "AMQP consumer stopped" << std::endl;
}

