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
    auto *channel = _channel;

    // install a monitor because the channel could be destructed
    Monitor monitor(channel);

    // report the size (technically, the channel object could be destructed now, but we ignore that case)
    if (_sizeCallback) _sizeCallback(messageCount);
    
    // we now know the name, so we can install the message callback on the channel
    _channel->install("", [channel, messageCallback](const Message &message, uint64_t deliveryTag, bool redelivered) {

        // install a monitor to deal with the case that the channel is removed
        Monitor monitor(channel);

        // call the callbacks
        if (messageCallback) messageCallback(message, deliveryTag, redelivered);
        
        // we can remove the callback now from the channel
        if (monitor.valid()) channel->uninstall("");
    });
    
    // return next object
    return _next;
}

/**
 *  Report success, although no message could be get
 */
Deferred *DeferredGet::reportSuccess() const
{
    // report the size
    if (_sizeCallback) _sizeCallback(0);

    // check if a callback was set
    if (_emptyCallback) _emptyCallback();
    
    // return next object
    return _next;
}

/**
 *  End of namespace
 */
}

