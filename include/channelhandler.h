#pragma once
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
     *  An error has occured on the channel
     *  The channel is no longer usable after an error has occured on it.
     *  @param  channel         the channel on which the error occured
     *  @param  message         human readable error message
     */
    virtual void onError(Channel *channel, const std::string &message) {}

    /**
     *  Method that is called when a queue is purged
     *  This is the result of a call to Channel::purgeQueue()
     *  @param  channel         the channel on which the queue was emptied
     *  @param  messageCount    number of message purged
     */
    virtual void onQueuePurged(Channel *channel, uint32_t messageCount) {}

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

};

/**
 *  End of namespace
 */
}
