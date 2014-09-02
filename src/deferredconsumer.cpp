/**
 *  DeferredConsumer.cpp
 *
 *  Implementation file for the DeferredConsumer class
 *
 *  @copyright 2014 Copernica BV
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
const std::shared_ptr<Deferred> &DeferredConsumer::reportSuccess(const std::string &name) const
{
    // we now know the name, so we can install the message callback on the channel
    _channel->install(name, _messageCallback);
    
    // skip if no special callback was installed
    if (!_consumeCallback) return Deferred::reportSuccess();
    
    // call the callback
    _consumeCallback(name);
    
    // return next object
    return _next;
}

/**
 *  End namespace
 */
}
