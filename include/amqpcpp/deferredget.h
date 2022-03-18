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
    inline DeferredGet &onSuccess(const MessageCallback& callback) { return onSuccess(MessageCallback(callback)); }
    DeferredGet &onSuccess(MessageCallback&& callback)
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
    inline DeferredGet &onError(const ErrorCallback& callback) { return onError(ErrorCallback(callback)); }
    DeferredGet &onError(ErrorCallback&& callback)
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
    inline DeferredGet &onReceived(const MessageCallback& callback) { return onReceived(MessageCallback(callback)); }
    DeferredGet &onReceived(MessageCallback&& callback)
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
    inline DeferredGet &onMessage(const MessageCallback& callback) { return onMessage(MessageCallback(callback)); }
    DeferredGet &onMessage(MessageCallback&& callback)
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
    inline DeferredGet &onEmpty(const EmptyCallback& callback) { return onEmpty(EmptyCallback(callback)); }
    DeferredGet &onEmpty(EmptyCallback&& callback)
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
    inline DeferredGet &onCount(const CountCallback& callback) { return onCount(CountCallback(callback)); }
    DeferredGet &onCount(CountCallback&& callback)
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
    inline DeferredGet &onBegin(const StartCallback& callback) { return onBegin(StartCallback(callback)); }
    DeferredGet &onBegin(StartCallback&& callback)
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
    inline DeferredGet &onStart(const StartCallback& callback) { return onStart(StartCallback(callback)); }
    DeferredGet &onStart(StartCallback&& callback)
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
    inline DeferredGet &onSize(const SizeCallback& callback) { return onSize(SizeCallback(callback)); }
    DeferredGet &onSize(SizeCallback&& callback)
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
    inline DeferredGet &onHeaders(const HeaderCallback& callback) { return onHeaders(HeaderCallback(callback)); }
    DeferredGet &onHeaders(HeaderCallback&& callback)
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
    inline DeferredGet &onData(const DataCallback& callback) { return onData(DataCallback(callback)); }
    DeferredGet &onData(DataCallback&& callback)
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
    inline DeferredGet &onComplete(const DeliveredCallback& callback) { return onComplete(DeliveredCallback(callback)); }
    DeferredGet &onComplete(DeliveredCallback&& callback)
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
    inline DeferredGet &onDelivered(const DeliveredCallback& callback) { return onDelivered(DeliveredCallback(callback)); }
    DeferredGet &onDelivered(DeliveredCallback&& callback)
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

