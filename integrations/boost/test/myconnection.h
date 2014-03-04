/**
 *  MyConnection.h
 *
 *
 *  @copyright 2014 Copernica BV
 *  @copyright 2014 TeamSpeak Systems GmbH
 */

#include "../boostnetworkhandler.h"

/**
 *  Class definition
 */
class MyConnection : 
    public AMQP::ConnectionHandler,
    public AMQP::ChannelHandler,
    public AMQP::BoostNetworkHandlerCallbacks
{
private:
    /**
     *  The actual TCP/SSl socket that is connected with RabbitMQ
     *  @var    TCP/SSL Socket
     */
    AMQP::BoostNetworkHandlerInterface* _networkHandler;
    
    /**
     *  The AMQP connection
     *  @var    Connection
     */
    AMQP::Connection *_connection;
    
    /**
     *  The AMQP channel
     *  @var    Channel
     */
    AMQP::Channel *_channel;

    /**
     *  Method that is called when the connection failed
     *  @param  connectionState state of the connection when the error occured
	 *  @param  ec error code
     */
    virtual void onIoError(AMQP::ConnectionState connectionState, boost::system::error_code& ec) override;
 
    /**
     *  Method that is called when the connection succeeded
     */
    virtual void onConnectionOpened() override;
    
    /**
     *  Method that is called when the socket is closed (as a result of a BoostNetworkHandlerInterface::closeConnection() call)
     */
    virtual void onConnectionClosed() override;


    /**
     *  Method that is called when the socket connect or read operation times out
     */
    virtual void onConnectionTimeout(AMQP::ConnectionState connectionState) override;

	/**
     *  Method that is called when data is received on the socket
     *  @param  buffer      Pointer to the fill input buffer
	 *  @param bytesAvailable how many bytes on the buffer
	 *  @return bytes used
     */
    virtual std::size_t onIoBytesRead(char *buffer, std::size_t bytesAvailable);
    
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
    virtual void onData(AMQP::Connection *connection, const char *buffer, size_t size) override;
    
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
    virtual void onError(AMQP::Connection *connection, const std::string &message) override;

    /**
     *  Method that is called when the login attempt succeeded. After this method
     *  was called, the connection is ready to use
     *
     *  @param  connection      The connection that can now be used
     */
    virtual void onConnected(AMQP::Connection *connection) override;

	 /**
     *  Method that is called when the connection was closed.
     * 
     *  This is the counter part of a call to Connection::close() and it confirms
     *  that the connection was correctly closed.
     * 
     *  @param  connection      The connection that was closed and that is now unusable
     */
    virtual void onClosed(AMQP::Connection *connection) override;
	
    /**
     *  Method that is called when the channel was succesfully created.
     *  Only after the channel was created, you can use it for subsequent messages over it
     *  @param  channel
     */
    virtual void onReady(AMQP::Channel *channel) override;
    
    /**
     *  An error has occured on the channel
     *  @param  channel
     *  @param  message
     */
    virtual void onError(AMQP::Channel *channel, const std::string &message) override;

    /**
     *  Method that is called when the channel was paused
     *  @param  channel
     */
    virtual void onPaused(AMQP::Channel *channel) override;
    
    /**
     *  Method that is called when the channel was resumed
     *  @param  channel
     */
    virtual void onResumed(AMQP::Channel *channel) override;
    
    /**
     *  Method that is called when a channel is closed
     *  @param  channel
     */
    virtual void onClosed(AMQP::Channel *channel) override;
    
    /**
     *  Method that is called when a transaction was started
     *  @param  channel
     */
    virtual void onTransactionStarted(AMQP::Channel *channel) override;
    
    /**
     *  Method that is called when a transaction was committed
     *  @param  channel
     */
    virtual void onTransactionCommitted(AMQP::Channel *channel) override;
    
    /**
     *  Method that is called when a transaction was rolled back
     *  @param  channel
     */
    virtual void onTransactionRolledBack(AMQP::Channel *channel) override;

    /**
     *  Method that is called when an exchange is bound
     *  @param  channel
     */
    virtual void onExchangeBound(AMQP::Channel *channel) override;
    
    /**
     *  Method that is called when an exchange is unbound
     *  @param  channel
     */
    virtual void onExchangeUnbound(AMQP::Channel *channel) override;
    
    /**
     *  Method that is called when an exchange is deleted
     *  @param  channel
     */
    virtual void onExchangeDeleted(AMQP::Channel *channel) override;
    
    /**
     *  Mehod that is called when an exchange is declared
     *  @param  channel
     */
    virtual void onExchangeDeclared(AMQP::Channel *channel) override;
    
    /**
     *  Method that is called when a queue is declared
     *  @param  channel
     *  @param  name            name of the queue
     *  @param  messageCount    number of messages in queue
     *  @param  consumerCount   number of active consumers
     */
    virtual void onQueueDeclared(AMQP::Channel *channel, const std::string &name, uint32_t messageCount, uint32_t consumerCount) override;
    
    /**
     *  Method that is called when a queue is bound
     *  @param  channel
     *  @param  
     */
    virtual void onQueueBound(AMQP::Channel *channel) override;
    
    /**
     *  Method that is called when a queue is deleted
     *  @param  channel
     *  @param  messageCount    number of messages deleted along with the queue
     */
    virtual void onQueueDeleted(AMQP::Channel *channel, uint32_t messageCount) override;
    
    /**
     *  Method that is called when a queue is unbound
     *  @param  channel
     */
    virtual void onQueueUnbound(AMQP::Channel *channel) override;
    
    /**
     *  Method that is called when a queue is purged
     *  @param  messageCount        number of message purged
     */
    virtual void onQueuePurged(AMQP::Channel *channel, uint32_t messageCount) override;

    /**
     *  Method that is called when the quality-of-service was changed
     *  This is the result of a call to Channel::setQos()
     */
    virtual void onQosSet(AMQP::Channel *channel) override;

    /**
     *  Method that is called when a consumer was started
     *  This is the result of a call to Channel::consume()
     *  @param  channel         the channel on which the consumer was started
     *  @param  tag             the consumer tag
     */
    virtual void onConsumerStarted(AMQP::Channel *channel, const std::string &tag) override;

    /**
     *  Method that is called when a consumer was stopped
     *  This is the result of a call to Channel::cancel()
     *  @param  channel         the channel on which the consumer was stopped
     *  @param  tag             the consumer tag
     */
    virtual void onConsumerStopped(AMQP::Channel *channel, const std::string &tag) override;
    
    /**
     *  Method that is called when a message has been received on a channel
     *  This message will be called for every message that is received after
     *  you started consuming. Make sure you acknowledge the messages when its
     *  safe to remove them from RabbitMQ (unless you set no-ack option when you
     *  started the consumer)
     *  @param  channel         the channel on which the consumer was started
     *  @param  message         the consumed message
     *  @param  deliveryTag     the delivery tag, you need this to acknowledge the message
     *  @param  consumerTag     the consumer identifier that was used to retrieve this message
     *  @param  redelivered     is this a redelivered message?
     */
    virtual void onReceived(AMQP::Channel *channel, const AMQP::Message &message, uint64_t deliveryTag, const std::string &consumerTag, bool redelivered) override;
    
    /**
     *  Method that is called when a message you tried to publish was returned
     *  by the server. This only happens when the 'mandatory' or 'immediate' flag
     *  was set with the Channel::publish() call.
     *  @param  channel         the channel on which the message was returned
     *  @param  message         the returned message
     *  @param  code            the reply code
     *  @param  text            human readable reply reason
     */
    virtual void onReturned(AMQP::Channel *channel, const AMQP::Message &message, int16_t code, const std::string &text) override;


public:
    /**
     *  Constructor
	 *  @param networkHandler
     *  @param ip
	 *  @param port port to connect to
     */
    MyConnection(AMQP::BoostNetworkHandlerInterface* networkHandler, const std::string &ip, std::uint16_t port);
    
    /**
     *  Destructor
     */
    virtual ~MyConnection();
};
