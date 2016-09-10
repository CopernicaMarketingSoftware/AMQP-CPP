/**
 *  DeferredGet.cpp
 *
 *  Implementation of the DeferredGet call
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2014 - 2016 Copernica BV
 */

/**
 *  Dependencies
 */
#include "includes.h"

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Report success for a get operation
 *
 *  @param  messagecount    Number of messages left in the queue
 *  @param  deliveryTag     Delivery tag of the message coming in
 *  @param  redelivered     Was the message redelivered?
 */
const std::shared_ptr<Deferred> &DeferredGet::reportSuccess(uint32_t messagecount, uint64_t deliveryTag, bool redelivered)
{
    // store delivery tag and redelivery status
    _deliveryTag = deliveryTag;
    _redelivered = redelivered;

    // install ourselves in the channel
    _channel->install("", shared_from_this(), true);

    // report the size (note that this is the size _minus_ the message that is retrieved
    // (and for which the callback will be called later), so it could be zero)
    if (_sizeCallback) _sizeCallback(messagecount);

    // return next handler
    return _next;
}

/**
 *  Report success, although no message could be gotten
 *  @return Deferred
 */
const std::shared_ptr<Deferred> &DeferredGet::reportSuccess() const
{
    // report the size
    if (_sizeCallback) _sizeCallback(0);

    // check if a callback was set
    if (_emptyCallback) _emptyCallback();

    // return next object
    return _next;
}

/**
 *  Announce that a message has been received
 *  @param  message The message to announce
 *  @param  deliveryTag The delivery tag (for ack()ing)
 *  @param  redelivered Is this a redelivered message
 */
void DeferredGet::announce(Message &&message, uint64_t deliveryTag, bool redelivered) const
{
    // monitor the channel
    Monitor monitor{ _channel };

    // the channel is now synchronized
    _channel->onSynchronized();

    // simply execute the message callback
    _messageCallback(std::move(message), deliveryTag, redelivered);

    // check if the channel is still valid
    if (!monitor.valid()) return;

    // stop consuming now
    _channel->uninstall({});
}

/**
 *  End of namespace
 */
}

