/**
 *  DeferredGet.h
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2014 - 2018 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include "deferredextreceiver.h"

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 *
 *  This class implements the 'shared_from_this' functionality, because
 *  it grabs a self-pointer when the callback is running, otherwise the onFinalize()
 *  is called before the actual message is consumed.
 */
class DeferredGet : public DeferredExtReceiver, public std::enable_shared_from_this<DeferredGet>
{
private:
    /**
     *  Callback in case the queue is empty
     *  @var    EmptyCallback
     */
    EmptyCallback _emptyCallback;

    /**
     *  Callback with the number of messages still in the queue
     *  @var    CountCallback
     */
    CountCallback _countCallback;

    /**
     *  Report success for a get operation
     *  @param  messagecount    Number of messages left in the queue
     *  @param  deliveryTag     Delivery tag of the message coming in
     *  @param  redelivered     Was the message redelivered?
     */
    virtual const std::shared_ptr<Deferred> &reportSuccess(uint32_t messagecount, uint64_t deliveryTag, bool redelivered) override;

    /**
     *  Report success when queue was empty
     *  @return Deferred
     */
    virtual const std::shared_ptr<Deferred> &reportSuccess() const override;

    /**
     *  Get reference to self to prevent that object falls out of scope
     *  @return std::shared_ptr
     */
    virtual std::shared_ptr<DeferredReceiver> lock() override { return shared_from_this(); }

    /**
     *  Extended implementation of the complete method that is called when a message was fully received
     */
    virtual void complete() override;

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
    DeferredGet(ChannelImpl *channel, bool failed = false) :
        DeferredExtReceiver(failed, channel) {}

public:
    /**
     *  Register a function to be called when a message arrives
     *  This fuction is also available as onReceived() and onMessage() because I always forget which name I gave to it
     *  @param  callback
     */
    DeferredGet &onSuccess(MessageCallback&&callback)
    {
        // store the callback
        _messageCallback = std::move(callback);

        // allow chaining
        return *this;
    }
    
    /**
     *  Register a function to be called when an error occurs. This should be defined, otherwise the base methods are used.
     *  @param  callback
     */
    DeferredGet &onError(ErrorCallback&&callback)
    {
        // store the callback
        _errorCallback = std::move(callback);

        // allow chaining
        return *this;
    }

    /**
     *  Register a function to be called when a message arrives
     *  This fuction is also available as onSuccess() and onMessage() because I always forget which name I gave to it
     *  @param  callback    the callback to execute
     */
    DeferredGet &onReceived(MessageCallback&&callback)
    {
        // store callback
        _messageCallback = std::move(callback);

        // allow chaining
        return *this;
    }

    /**
     *  Register a function to be called when a message arrives
     *  This fuction is also available as onSuccess() and onReceived() because I always forget which name I gave to it
     *  @param  callback    the callback to execute
     */
    DeferredGet &onMessage(MessageCallback&&callback)
    {
        // store callback
        _messageCallback = std::move(callback);

        // allow chaining
        return *this;
    }

    /**
     *  Register a function to be called if no message could be fetched
     *  @param  callback    the callback to execute
     */
    DeferredGet &onEmpty(EmptyCallback&&callback)
    {
        // store callback
        _emptyCallback = std::move(callback);

        // allow chaining
        return *this;
    }

    /**
     *  Register a function to be called when queue size information is known
     *  @param  callback    the callback to execute
     */
    DeferredGet &onCount(CountCallback&&callback)
    {
        // store callback
        _countCallback = std::move(callback);

        // allow chaining
        return *this;
    }

    /**
     *  Register the function to be called when a new message is expected
     *
     *  @param  callback    The callback to invoke
     *  @return Same object for chaining
     */
    DeferredGet &onBegin(StartCallback&&callback)
    {
        // store callback
        _startCallback = std::move(callback);

        // allow chaining
        return *this;
    }

    /**
     *  Register the function to be called when a new message is expected
     *
     *  @param  callback    The callback to invoke
     *  @return Same object for chaining
     */
    DeferredGet &onStart(StartCallback&&callback)
    {
        // store callback
        _startCallback = std::move(callback);

        // allow chaining
        return *this;
    }

    /**
     *  Register a function that is called when the message size is known
     * 
     *  @param  callback    The callback to invoke for message headers
     *  @return Same object for chaining
     */
    DeferredGet &onSize(SizeCallback&&callback)
    {
        // store callback
        _sizeCallback = std::move(callback);
        
        // allow chaining
        return *this;
    }

    /**
     *  Register the function to be called when message headers come in
     *
     *  @param  callback    The callback to invoke for message headers
     *  @return Same object for chaining
     */
    DeferredGet &onHeaders(HeaderCallback&&callback)
    {
        // store callback
        _headerCallback = std::move(callback);

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
    DeferredGet &onData(DataCallback&&callback)
    {
        // store callback
        _dataCallback = std::move(callback);

        // allow chaining
        return *this;
    }

    /**
     *  Register a funtion to be called when a message was completely received
     *
     *  @param  callback    The callback to invoke
     *  @return Same object for chaining
     */
    DeferredGet &onComplete(DeliveredCallback&&callback)
    {
        // store callback
        _deliveredCallback = std::move(callback);

        // allow chaining
        return *this;
    }

    /**
     *  Register a funtion to be called when a message was completely received
     *
     *  @param  callback    The callback to invoke
     *  @return Same object for chaining
     */
    DeferredGet &onDelivered(DeliveredCallback&&callback)
    {
        // store callback
        _deliveredCallback = std::move(callback);

        // allow chaining
        return *this;
    }
};

/**
 *  End of namespace
 */
}

