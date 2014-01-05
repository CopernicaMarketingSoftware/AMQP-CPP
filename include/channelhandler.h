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
     *  @param  channel
     */
    virtual void onReady(Channel *channel) {}
    
    /**
     *  An error has occured on the channel
     *  @param  channel
     *  @param  message
     */
    virtual void onError(Channel *channel, const std::string &message) {}

    /**
     *  Method that is called when the channel was paused
     *  This is the result of a call to Channel::pause()
     *  @param  channel
     */
    virtual void onPaused(Channel *channel) {}
    
    /**
     *  Method that is called when the channel was resumed
     *  This is the result of a call to Channel::resume()
     *  @param  channel
     */
    virtual void onResumed(Channel *channel) {}
    
    /**
     *  Method that is called when a channel is closed
     *  This is the result of a call to Channel::close()
     *  @param  channel
     */
    virtual void onClosed(Channel *channel) {}
    
    /**
     *  Method that is called when a transaction was started
     *  This is the result of a call to Channel::startTransaction()
     *  @param  channel
     */
    virtual void onTransactionStarted(Channel *channel) {}
    
    /**
     *  Method that is called when a transaction was committed
     *  This is the result of a call to Channel::commitTransaction()
     *  @param  channel
     */
    virtual void onTransactionCommitted(Channel *channel) {}
    
    /**
     *  Method that is called when a transaction was rolled back
     *  This is the result of a call to Channel::rollbackTransaction()
     *  @param  channel
     */
    virtual void onTransactionRolledBack(Channel *channel) {}
    
    /**
     *  Method that is called when an exchange is bound
     *  This is the result of a call to Channel::bindExchange()
     *  @param  channel
     */
    virtual void onExchangeBound(Channel *channel) {}
    
    /**
     *  Method that is called when an exchange is unbound
     *  This is the result of a call to Channel::unbindExchange()
     *  @param  channel
     */
    virtual void onExchangeUnbound(Channel *channel) {}     
    
    /**
     *  Method that is called when an exchange is deleted
     *  This is the result of a call to Channel::deleteExchange()
     *  @param  channel
     */
    virtual void onExchangeDeleted(Channel *channel) {}
    
    /**
     *  Mehod that is called when an exchange is declared
     *  This is the result of a call to Channel::declareExchange()
     *  @param  channel
     */
    virtual void onExchangeDeclared(Channel *channel) {}
    
    /**
     *  Method that is called when a queue is declared
     *  This is the result of a call to Channel::declareQueue()
     *  @param  channel
     *  @param  name            name of the queue
     *  @param  messageCount    number of messages in queue
     *  @param  consumerCount   number of active consumers
     */
    virtual void onQueueDeclared(Channel *channel, const std::string &name, uint32_t messageCount, uint32_t consumerCount) {}
    
    /**
     *  Method that is called when a queue is bound
     *  This is the result of a call to Channel::bindQueue()
     *  @param  channel
     *  @param  
     */
    virtual void onQueueBound(Channel *channel) {}
    
    /**
     *  Method that is called when a queue is deleted
     *  This is the result of a call to Channel::deleteQueue()
     *  @param  channel
     *  @param  messageCount    number of messages deleted along with the queue
     */
    virtual void onQueueDeleted(Channel *channel, uint32_t messageCount) {}
    
    /**
     *  Method that is called when a queue is unbound
     *  This is the result of a call to Channel::unbindQueue()
     *  @param  channel
     */
    virtual void onQueueUnbound(Channel *channel) {}
    
    /**
     *  Method that is called when a queue is purged
     *  This is the result of a call to Channel::purgeQueue()
     *  @param  messageCount        number of message purged
     */
    virtual void onQueuePurged(Channel *channel, uint32_t messageCount) {}

    /**
     *  Method that is called when the quality-of-service was changed
     *  This is the result of a call to Channel::setQos()
     */
    virtual void onQosSet(Channel *channel) {}

};

/**
 *  End of namespace
 */
}
