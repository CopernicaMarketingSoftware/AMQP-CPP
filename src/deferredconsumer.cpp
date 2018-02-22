/**
 *  DeferredConsumer.cpp
 *
 *  Implementation file for the DeferredConsumer class
 *
 *  @copyright 2014 - 2017 Copernica BV
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
void DeferredConsumer::announce(const Message &message, uint64_t deliveryTag, bool redelivered) const
{
    // simply execute the message callback
    _messageCallback(message, deliveryTag, redelivered);
}

/**
 *  Announce that a message has been returned
 *  @param returnCode The return code
 *  @param returnText The return text
 *  @param exchange The exchange the message was published to
 *  @param routingKey The routing key
 *  @param message The returned message
 */
void DeferredConsumer::announce_return(int16_t replyCode, const std::string &replyText, const std::string &exchange, const std::string &routingKey, const Message &message) const
{
	if (_returnCallback)
	{
    	// simply execute the return callback
    	_returnCallback(replyCode, replyText, exchange, routingKey, message);
	}
}

/**
 *  Register a function to be called when a message was unroutable by the server
 *
 *  @param callback The callback to invoke
 *  @return Same object for chaining
 */
DeferredConsumer &DeferredConsumer::onReturn(const ReturnCallback &callback)
{
    // store callback
    _returnCallback = callback;

    // allow chaining
    return *this;
}

/**
 *  End namespace
 */
}
