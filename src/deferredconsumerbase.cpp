/**
 *  deferredconsumerbase.cpp
 *
 *  Base class for the deferred consumer and the
 *  deferred get.
 *
 *  @copyright 2016 Copernica B.V.
 */

/**
 *  Dependencies
 */
#include "../include/deferredconsumerbase.h"
#include "basicdeliverframe.h"
#include "basicgetokframe.h"
#include "basicheaderframe.h"
#include "bodyframe.h"

/**
 *  Start namespace
 */
namespace AMQP {

/**
 *  Process a delivery frame
 *
 *  @param  frame   The frame to process
 */
void DeferredConsumerBase::process(BasicDeliverFrame &frame)
{
    // retrieve the delivery tag and whether we were redelivered
    _deliveryTag = frame.deliveryTag();
    _redelivered = frame.redelivered();

    // anybody interested in the new message?
    if (_beginCallback)   _beginCallback();

    // do we have anybody interested in messages?
    if (_messageCallback) _message.construct(frame.exchange(), frame.routingKey());
}

/**
 *  Process a delivery frame from a get request
 *
 *  @param  frame   The frame to process
 */
void DeferredConsumerBase::process(BasicGetOKFrame &frame)
{
    // retrieve the delivery tag and whether we were redelivered
    _deliveryTag = frame.deliveryTag();
    _redelivered = frame.redelivered();

    // anybody interested in the new message?
    if (_beginCallback)   _beginCallback();

    // do we have anybody interested in messages?
    if (_messageCallback) _message.construct(frame.exchange(), frame.routingKey());
}

/**
 *  Process the message headers
 *
 *  @param  frame   The frame to process
 */
void DeferredConsumerBase::process(BasicHeaderFrame &frame)
{
    // store the body size
    _bodySize = frame.bodySize();

    // do we have a message?
    if (_message)
    {
        // store the body size and metadata
        _message->setBodySize(_bodySize);
        _message->set(frame.metaData());
    }

    // anybody interested in the headers?
    if (_headerCallback) _headerCallback(frame.metaData());

    // no body data expected? then we are now complete
    if (!_bodySize) complete();
}

/**
 *  Process the message data
 *
 *  @param  frame   The frame to process
 */
void DeferredConsumerBase::process(BodyFrame &frame)
{
    // make sure we stay in scope
    auto self = shared_from_this();

    // update the bytes still to receive
    _bodySize -= frame.payloadSize();

    // anybody interested in the data?
    if (_dataCallback) _dataCallback(frame.payload(), frame.payloadSize());

    // do we have a message? then append the data
    if (_message) _message->append(frame.payload(), frame.payloadSize());

    // if all bytes were received we are now complete
    if (!_bodySize) complete();
}

/**
 *  Indicate that a message was done
 */
void DeferredConsumerBase::complete()
{
    // make sure we stay in scope
    auto self = shared_from_this();

    // also monitor the channel
    Monitor monitor{ _channel };

    // do we have a message?
    if (_message)
    {
        // announce the message
        announce(std::move(*_message), _deliveryTag, _redelivered);

        // and destroy it
        _message.reset();
    }

    // do we have to inform anyone about completion?
    if (_completeCallback) _completeCallback(_deliveryTag, _redelivered);

    // do we still have a valid channel
    if (!monitor.valid()) return;

    // we are now done executing
    _channel->complete();
}

/**
 *  End namespace
 */
}
