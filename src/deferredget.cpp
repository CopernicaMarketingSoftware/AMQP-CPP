/**
 *  DeferredGet.cpp
 *
 *  Implementation of the DeferredGet call
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2014 Copernica BV
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
 *  Report success, a get message succeeded and the message is expected soon
 *  @param  messageCount    Message count
 *  @return Deferred
 */
Deferred *DeferredGet::reportSuccess(uint32_t messageCount) const
{
    // make copies of the callbacks
    auto messageCallback = _messageCallback;
    auto finalizeCallback = _finalizeCallback;
    auto *channel = _channel;
    
    // we now know the name, so we can install the message callback on the channel
    _channel->install("", [channel, messageCallback, finalizeCallback](const Message &message, uint64_t deliveryTag, bool redelivered) {

        // we can remove the callback now from the channel
        channel->uninstall("");
        
        // call the callbacks
        if (messageCallback) messageCallback(message, deliveryTag, redelivered);
        
        // call the finalize callback
        if (finalizeCallback) finalizeCallback();
    });
    
    // return next object
    return _next;
}

/**
 *  Report success, although no message could be get
 */
Deferred *DeferredGet::reportSuccess() const
{
    // check if a callback was set
    if (_emptyCallback) _emptyCallback();
    
    // call finalize callback
    if (_finalizeCallback) _finalizeCallback();
    
    // return next object
    return _next;
}

/**
 *  End of namespace
 */
}

