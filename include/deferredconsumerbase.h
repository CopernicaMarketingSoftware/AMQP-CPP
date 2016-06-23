/**
 *  deferredconsumerbase.h
 *
 *  Base class for the deferred consumer and the
 *  deferred get.
 *
 *  @copyright 2016 Copernica B.V.
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include "deferred.h"
#include "stack_ptr.h"
#include "message.h"

/**
 *  Start namespace
 */
namespace AMQP {

/**
 *  Forward declarations
 */
class BasicDeliverFrame;
class BasicHeaderFrame;
class BodyFrame;

/**
 *  Base class for deferred consumers
 */
class DeferredConsumerBase :
    public Deferred,
    public std::enable_shared_from_this<DeferredConsumerBase>
{
private:
    /**
     *  Size of the body of the current message
     *  @var    uint64_t
     */
    uint64_t _bodySize = 0;

    /**
     *  Process a delivery frame
     *
     *  @param  frame   The frame to process
     */
    void process(BasicDeliverFrame &frame);

    /**
     *  Process the message headers
     *
     *  @param  frame   The frame to process
     */
    void process(BasicHeaderFrame &frame);

    /**
     *  Process the message data
     *
     *  @param  frame   The frame to process
     */
    void process(BodyFrame &frame);

    /**
     *  Indicate that a message was done
     */
    void complete();

    /**
     *  Emit a message
     *
     *  @param  message     The message to emit
     *  @param  deliveryTag The delivery tag (for ack()ing)
     *  @param  redelivered Is this a redelivered message
     */
    virtual void emit(Message &&message, uint64_t deliveryTag, bool redelivered) const = 0;

    /**
     *  Frames may be processed
     */
    friend class ChannelImpl;
    friend class BasicDeliverFrame;
    friend class BasicHeaderFrame;
    friend class BodyFrame;
protected:
    /**
     *  The delivery tag for the current message
     *  @var    uint64_t
     */
    uint64_t _deliveryTag = 0;

    /**
     *  Is this a redelivered message
     *  @var    bool
     */
    bool _redelivered = false;

    /**
     *  The channel to which the consumer is linked
     *  @var    ChannelImpl
     */
    ChannelImpl *_channel;

    /**
     *  Callback for new message
     *  @var    BeginCallback
     */
    BeginCallback _beginCallback;

    /**
     *  Callback for incoming headers
     *  @var    HeaderCallback
     */
    HeaderCallback _headerCallback;

    /**
     *  Callback for when a chunk of data comes in
     *  @var    DataCallback
     */
    DataCallback _dataCallback;

    /**
     *  Callback for incoming messages
     *  @var    MessageCallback
     */
    MessageCallback _messageCallback;

    /**
     *  Callback for when a message was complete finished
     *  @var    CompleteCallback
     */
    CompleteCallback _completeCallback;

    /**
     *  The message that we are currently receiving
     *  @var    stack_ptr<Message>
     */
    stack_ptr<Message> _message;

    /**
     *  Constructor
     *
     *  @param  failed  Have we already failed?
     *  @param  channel The channel we are consuming on
     */
    DeferredConsumerBase(bool failed, ChannelImpl *channel) : Deferred(failed), _channel(channel) {}
public:
    /**
     *  Register the function to be called when a new message is expected
     *
     *  @param  callback    The callback to invoke
     *  @return Same object for chaining
     */
    DeferredConsumerBase &onBegin(const BeginCallback &callback)
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
    DeferredConsumerBase &onHeaders(const HeaderCallback &callback)
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
    DeferredConsumerBase &onData(const DataCallback &callback)
    {
        // store callback
        _dataCallback = callback;

        // allow chaining
        return *this;
    }

    /**
     *  Register a function to be called when a message arrives
     *  This fuction is also available as onSuccess() and onMessage() because I always forget which name I gave to it
     *  @param  callback    the callback to execute
     */
    DeferredConsumerBase &onReceived(const MessageCallback &callback)
    {
        // store callback
        _messageCallback = callback;

        // allow chaining
        return *this;
    }

    /**
     *  Register a function to be called when a message arrives
     *  This fuction is also available as onSuccess() and onReceived() because I always forget which name I gave to it
     *  @param  callback    the callback to execute
     */
    DeferredConsumerBase &onMessage(const MessageCallback &callback)
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
    DeferredConsumerBase& onComplete(const CompleteCallback &callback)
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
