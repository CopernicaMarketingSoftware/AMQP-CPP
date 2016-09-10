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
class BasicGetOKFrame;
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
     *  Process a delivery frame from a get request
     *
     *  @param  frame   The frame to process
     */
    void process(BasicGetOKFrame &frame);

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
     *  Announce that a message has been received
     *  @param  message     The message to announce
     *  @param  deliveryTag The delivery tag (for ack()ing)
     *  @param  redelivered Is this a redelivered message
     */
    virtual void announce(Message &&message, uint64_t deliveryTag, bool redelivered) const = 0;

    /**
     *  Frames may be processed
     */
    friend class ChannelImpl;
    friend class BasicDeliverFrame;
    friend class BasicGetOKFrame;
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
};

/**
 *  End namespace
 */
}
