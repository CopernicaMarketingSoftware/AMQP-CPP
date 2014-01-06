/**
 *  ChannelHandler.h
 *
 *  Interface that should be implemented by a user of the AMQP library,
 *  and that is passed to the Connection::createChannel() method.
 *
 *  This interface contains a number of methods that are called when
 *  the channel changes state.
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class ChannelHandler
{
public:
    /**
     *  Method that is called when the channel was succesfully created.
     *  @param  channel         the channel that is ready
     */
    virtual void onReady(Channel *channel) {}
    
    /**
     *  An error has occured on the channel
     *  The channel is no longer usable after an error has occured on it.
     *  @param  channel         the channel on which the error occured
     *  @param  message         human readable error message
     */
    virtual void onError(Channel *channel, const std::string &message) {}

    /**
     *  Method that is called when the channel was paused
     *  This is the result of a call to Channel::pause()
     *  @param  channel         the channel that is now paused
     */
    virtual void onPaused(Channel *channel) {}
    
    /**
     *  Method that is called when the channel was resumed
     *  This is the result of a call to Channel::resume()
     *  @param  channel         the channel that is no longer paused
     */
    virtual void onResumed(Channel *channel) {}
    
    /**
     *  Method that is called when a channel is closed
     *  This is the result of a call to Channel::close()
     *  @param  channel         the channel that is closed
     */
    virtual void onClosed(Channel *channel) {}
    
    /**
     *  Method that is called when a transaction was started
     *  This is the result of a call to Channel::startTransaction()
     *  @param  channel         the channel on which the transaction was started
     */
    virtual void onTransactionStarted(Channel *channel) {}
    
    /**
     *  Method that is called when a transaction was committed
     *  This is the result of a call to Channel::commitTransaction()
     *  @param  channel         the channel on which the transaction was committed
     */
    virtual void onTransactionCommitted(Channel *channel) {}
    
    /**
     *  Method that is called when a transaction was rolled back
     *  This is the result of a call to Channel::rollbackTransaction()
     *  @param  channel         the channel on which the transaction was rolled back
     */
    virtual void onTransactionRolledBack(Channel *channel) {}
    
    /**
     *  Method that is called when an exchange is bound
     *  This is the result of a call to Channel::bindExchange()
     *  @param  channel         the channel on which the exchange was bound
     */
    virtual void onExchangeBound(Channel *channel) {}
    
    /**
     *  Method that is called when an exchange is unbound
     *  This is the result of a call to Channel::unbindExchange()
     *  @param  channel         the channel on which the exchange was unbound
     */
    virtual void onExchangeUnbound(Channel *channel) {}     
    
    /**
     *  Method that is called when an exchange is deleted
     *  This is the result of a call to Channel::deleteExchange()
     *  @param  channel         the channel on which the exchange was deleted
     */
    virtual void onExchangeDeleted(Channel *channel) {}
    
    /**
     *  Mehod that is called when an exchange is declared
     *  This is the result of a call to Channel::declareExchange()
     *  @param  channel         the channel on which the exchange was declared
     */
    virtual void onExchangeDeclared(Channel *channel) {}
    
    /**
     *  Method that is called when a queue is declared
     *  This is the result of a call to Channel::declareQueue()
     *  @param  channel         the channel on which the queue was declared
     *  @param  name            name of the queue
     *  @param  messageCount    number of messages in queue
     *  @param  consumerCount   number of active consumers
     */
    virtual void onQueueDeclared(Channel *channel, const std::string &name, uint32_t messageCount, uint32_t consumerCount) {}
    
    /**
     *  Method that is called when a queue is bound
     *  This is the result of a call to Channel::bindQueue()
     *  @param  channel         the channel on which the queue was bound
     */
    virtual void onQueueBound(Channel *channel) {}
    
    /**
     *  Method that is called when a queue is deleted
     *  This is the result of a call to Channel::deleteQueue()
     *  @param  channel         the channel on which the queue was deleted
     *  @param  messageCount    number of messages deleted along with the queue
     */
    virtual void onQueueDeleted(Channel *channel, uint32_t messageCount) {}
    
    /**
     *  Method that is called when a queue is unbound
     *  This is the result of a call to Channel::unbindQueue()
     *  @param  channel         the channel on which the queue was unbound
     */
    virtual void onQueueUnbound(Channel *channel) {}
    
    /**
     *  Method that is called when a queue is purged
     *  This is the result of a call to Channel::purgeQueue()
     *  @param  channel         the channel on which the queue was emptied
     *  @param  messageCount    number of message purged
     */
    virtual void onQueuePurged(Channel *channel, uint32_t messageCount) {}

    /**
     *  Method that is called when the quality-of-service was changed
     *  This is the result of a call to Channel::setQos()
     *  @param  channel         the channel on which the qos was set
     */
    virtual void onQosSet(Channel *channel) {}
    
    /**
     *  Method that is called when a consumer was started
     *  This is the result of a call to Channel::consume()
     *  @param  channel         the channel on which the consumer was started
     *  @param  tag             the consumer tag
     */
    virtual void onConsumerStarted(Channel *channel, const std::string &tag) {}

    /**
     *  Method that is called when a consumer was stopped
     *  This is the result of a call to Channel::cancel()
     *  @param  channel         the channel on which the consumer was stopped
     *  @param  tag             the consumer tag
     */
    virtual void onConsumerStopped(Channel *channel, const std::string &tag) {}
    
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
    virtual void onReceived(Channel *channel, const Message &message, uint64_t deliveryTag, const std::string &consumerTag, bool redelivered) {}
    
    /**
     *  Method that is called when a message you tried to publish was returned
     *  by the server. This only happens when the 'mandatory' or 'immediate' flag
     *  was set with the Channel::publish() call.
     *  @param  channel         the channel on which the message was returned
     *  @param  message         the returned message
     *  @param  code            the reply code
     *  @param  text            human readable reply reason
     */
    virtual void onReturned(Channel *channel, const Message &message, int16_t code, const std::string &text) {}
    
    /**
     *  Method that is called when the server starts recovering messages
     *  This is the result of a call to Channel::recover()
     *  @param  channel         the channel on which the recover method was called
     */
    virtual void onRecovering(Channel *channel) {}

};

/**
 *  End of namespace
 */
}
