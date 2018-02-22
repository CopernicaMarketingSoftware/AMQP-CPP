/**
 *  DeferredConsumer.h
 *
 *  Deferred callback for consumers
 *
 *  @copyright 2014 - 2017 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include "deferredconsumerbase.h"

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  We extend from the default deferred and add extra functionality
 */
class DeferredConsumer : public DeferredConsumerBase
{
private:
    /**
     *  Callback to execute when consumption has started
     *  @var    ConsumeCallback
     */
    ConsumeCallback _consumeCallback;

    /**
     *  Report success for frames that report start consumer operations
     *  @param  name            Consumer tag that is started
     *  @return Deferred
     */
    virtual const std::shared_ptr<Deferred> &reportSuccess(const std::string &name) override;

    /**
     *  Announce that a message has been received
     *  @param  message The message to announce
     *  @param  deliveryTag The delivery tag (for ack()ing)
     *  @param  redelivered Is this a redelivered message
     */
    virtual void announce(const Message &message, uint64_t deliveryTag, bool redelivered) const override;

    /**
     *  Announce that a message has been returned
     *  @param replyCode The reply code
     *  @param replyText The reply text
     *  @param exchange The exchange the message was published to
     *  @param routingKey The routing key
     *  @param message The returned message
     */
    virtual void announce_return(int16_t replyCode, const std::string &replyText, const std::string &exchange, const std::string &routingKey, const Message &message) const override;

    /**
     *  The channel implementation may call our
     *  private members and construct us
     */
    friend class ChannelImpl;
    friend class ConsumedMessage;

public:
    /**
     *  Protected constructor that can only be called
     *  from within the channel implementation
     *
     *  Note: this constructor _should_ be protected, but because make_shared
     *  will then not work, we have decided to make it public after all,
     *  because the work-around would result in not-so-easy-to-read code.
     *
     *  @param  channel     the channel implementation
     *  @param  failed      are we already failed?
     */
    DeferredConsumer(ChannelImpl *channel, bool failed = false) :
        DeferredConsumerBase(failed, channel) {}

public:
    /**
     *  Register a callback function that gets called when the consumer is
     *  started. In the callback you will for receive the consumer-tag
     *  that you need to later stop the consumer
     *  @param  callback
     */
    DeferredConsumer &onSuccess(const ConsumeCallback &callback)
    {
        // store the callback
        _consumeCallback = callback;

        // allow chaining
        return *this;
    }

    /**
     *  Register the function that is called when the consumer starts.
     *  It is recommended to use the onSuccess() method mentioned above
     *  since that will also pass the consumer-tag as parameter.
     *  @param  callback
     */
    DeferredConsumer &onSuccess(const SuccessCallback &callback)
    {
        // call base
        Deferred::onSuccess(callback);

        // allow chaining
        return *this;
    }

    /**
     *  Register a function to be called when a full message is received
     *  @param  callback    the callback to execute
     */
    DeferredConsumer &onReceived(const MessageCallback &callback)
    {
        // store callback
        _messageCallback = callback;

        // allow chaining
        return *this;
    }

    /**
     *  Alias for onReceived() (see above)
     *  @param  callback    the callback to execute
     */
    DeferredConsumer &onMessage(const MessageCallback &callback)
    {
        // store callback
        _messageCallback = callback;

        // allow chaining
        return *this;
    }

    /**
     *  RabbitMQ sends a message in multiple frames to its consumers.
     *  The AMQP-CPP library collects these frames and merges them into a 
     *  single AMQP::Message object that is passed to the callback that
     *  you can set with the onReceived() or onMessage() methods (see above).
     * 
     *  However, you can also write your own algorithm to merge the frames.
     *  In that case you can install callbacks to handle the frames. Every
     *  message is sent in a number of frames:
     * 
     *      - a begin frame that marks the start of the message
     *      - an optional header if the message was sent with an envelope
     *      - zero or more data frames (usually 1, but more for large messages)
     *      - an end frame to mark the end of the message.
     *  
     *  To install handlers for these frames, you can use the onBegin(), 
     *  onHeaders(), onData() and onComplete() methods.
     * 
     *  If you just rely on the onReceived() or onMessage() callbacks, you
     *  do not need any of the methods below this line.
     */

    /**
     *  Register the function that is called when the start frame of a new 
     *  consumed message is received
     *
     *  @param  callback    The callback to invoke
     *  @return Same object for chaining
     */
    DeferredConsumer &onBegin(const BeginCallback &callback)
    {
        // store callback
        _beginCallback = callback;

        // allow chaining
        return *this;
    }

    /**
     *  Register the function that is called when message headers come in
     *
     *  @param  callback    The callback to invoke for message headers
     *  @return Same object for chaining
     */
    DeferredConsumer &onHeaders(const HeaderCallback &callback)
    {
        // store callback
        _headerCallback = callback;

        // allow chaining
        return *this;
    }

    /**
     *  Register the function to be called when a chunk of data comes in
     *
     *  Note that this function may be called zero, one or multiple times
     *  for each incoming message depending on the size of the message data.
     *
     *  If you install this callback you very likely also want to install
     *  the onComplete callback so you know when the last data part was
     *  received.
     *
     *  @param  callback    The callback to invoke for chunks of message data
     *  @return Same object for chaining
     */
    DeferredConsumer &onData(const DataCallback &callback)
    {
        // store callback
        _dataCallback = callback;

        // allow chaining
        return *this;
    }

    /**
     *  Register a funtion to be called when a message was completely received
     *
     *  @param  callback    The callback to invoke
     *  @return Same object for chaining
     */
    DeferredConsumer &onComplete(const CompleteCallback &callback)
    {
        // store callback
        _completeCallback = callback;

        // allow chaining
        return *this;
    }

    /**
     *  Register a function to be called when a message was unroutable by the server
     *
     *  @param callback The callback to invoke
     *  @return Same object for chaining
     */
    DeferredConsumer &onReturn(const ReturnCallback &callback);
};

/**
 *  End namespace
 */
}
