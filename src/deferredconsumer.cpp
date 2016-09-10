/**
 *  DeferredConsumer.cpp
 *
 *  Implementation file for the DeferredConsumer class
 *
 *  @copyright 2014 - 2016 Copernica BV
 */
#include "includes.h"

/**
 *  Namespace
 */
namespace AMQP {

/**
 *  Report success for frames that report start consumer operations
 *  @param  name            Consumer tag that is started
 *  @return Deferred
 */
const std::shared_ptr<Deferred> &DeferredConsumer::reportSuccess(const std::string &name)
{
    // we now know the name, so install ourselves in the channel
    _channel->install(name, shared_from_this());

    // skip if no special callback was installed
    if (!_consumeCallback) return Deferred::reportSuccess();

    // call the callback
    _consumeCallback(name);

    // return next object
    return _next;
}

/**
 *  Announce that a message was received
 *  @param  message     The message to announce
 *  @param  deliveryTag The delivery tag (for ack()ing)
 *  @param  redelivered Is this a redelivered message
 */
void DeferredConsumer::announce(Message &&message, uint64_t deliveryTag, bool redelivered) const
{
    // simply execute the message callback
    _messageCallback(std::move(message), deliveryTag, redelivered);
}

/**
 *  End namespace
 */
}
