/**
 *  DeferredConsumer.h
 *
 *  Deferred callback for consumers
 *
 *  @copyright 2014 - 2016 Copernica BV
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
    virtual void announce(Message &&message, uint64_t deliveryTag, bool redelivered) const override;

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
     *  Register the function to be called when a new message is expected
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
     *  Register the function to be called when message headers come in
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
     *  Register the function that is called when the consumer starts
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
     *  Register the function that is called when the consumer starts
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
     *  Register a function to be called when a message arrives
     *  This fuction is also available as onMessage() because I always forget which name I gave to it
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
     *  Register a function to be called when a message arrives
     *  This fuction is also available as onReceived() because I always forget which name I gave to it
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
};

/**
 *  End namespace
 */
}
