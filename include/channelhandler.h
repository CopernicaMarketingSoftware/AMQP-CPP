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
     *  Only after the channel was created, you can use it for subsequent messages over it
     *  @param  channel
     */
    virtual void onReady(Channel *channel) {}
    
    /**
     *  An error has occured on the channel
     *  @param  channel
     *  @param  message
     */
    virtual void onChannelError(Channel *channel, const std::string &message) {}

    /**
     *  Method that is called when the channel was paused
     *  @param  channel
     */
    virtual void onPaused(Channel *channel) {}
    
    /**
     *  Method that is called when the channel was resumed
     *  @param  channel
     */
    virtual void onResumed(Channel *channel) {}
    
    /**
     *  Method that is called when a channel is closed
     *  @param  channel
     */
    virtual void onClosed(Channel *channel) {}
    
    /**
     *  Method that is called when a transaction was started
     *  @param  channel
     */
    virtual void onTransactionStarted(Channel *channel) {}
    
    /**
     *  Method that is called when a transaction was committed
     *  @param  channel
     */
    virtual void onTransactionCommitted(Channel *channel) {}
    
    /**
     *  Method that is called when a transaction was rolled back
     *  @param  channel
     */
    virtual void onTransactionRolledBack(Channel *channel) {}
    
    /**
     *  Method that is called when an exchange is bound
     *  @param  channel
     */
    virtual void onExchangeBound(Channel *channel) {}
    
    /**
     *  Method that is called when an exchange is unbound
     *  @param  channel
     */
    virtual void onExchangeUnbound(Channel *channel) {}     
    
    /**
     *  Method that is called when an exchange is deleted
     *  @param  channel
     */
    virtual void onExchangeDeleted(Channel *channel) {}
    
    /**
     *  Mehod that is called when an exchange is declared
     *  @param  channel
     */
    virtual void onExchangeDeclared(Channel *channel) {}
    
    /**
     *  Method that is called when a queue is declared
     *  @param  channel
     *  @param  name            name of the queue
     *  @param  messageCount    number of messages in queue
     *  @param  consumerCount   number of active consumers
     */
    virtual void onQueueDeclared(Channel *channel, const std::string &name, uint32_t messageCount, uint32_t consumerCount) {}
    
    /**
     *  Method that is called when a queue is bound
     *  @param  channel
     *  @param  
     */
    virtual void onQueueBound(Channel *channel) {}
    
    /**
     *  Method that is called when a queue is deleted
     *  @param  channel
     *  @param  messageCount    number of messages deleted along with the queue
     */
    virtual void onQueueDeleted(Channel *channel, uint32_t messageCount) {}
    
    /**
     *  Method that is called when a queue is unbound
     *  @param  channel
     */
    virtual void onQueueUnbound(Channel *channel) {}
    
    /**
     *  Method that is called when a queue is purged
     *  @param  messageCount        number of message purged
     */
    virtual void onQueuePurged(Channel *channel, uint32_t messageCount) {}

};

/**
 *  End of namespace
 */
}
