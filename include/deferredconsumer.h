/**
 *  DeferredConsumer.h
 *
 *  Deferred callback for consumers
 *
 *  @copyright 2014 Copernica BV
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
     *  Emit a message
     *
     *  @param  message The message to emit
     *  @param  deliveryTag The delivery tag (for ack()ing)
     *  @param  redelivered Is this a redelivered message
     */
    void emit(Message &&message, uint64_t deliveryTag, bool redelivered) const override;

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
};

/**
 *  End namespace
 */
}
