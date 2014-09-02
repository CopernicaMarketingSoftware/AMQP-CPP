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
const std::shared_ptr<Deferred> &DeferredGet::reportSuccess(uint32_t messageCount) const
{
    // we grab a self pointer to ensure that the deferred object stays alive
    auto self = shared_from_this();

    // report the size (technically, the channel object could be destructed now, but we ignore that case)
    if (_sizeCallback) _sizeCallback(messageCount);
    
    // we now know the name, so we can install the message callback on the channel
    _channel->install("", [self, this](const Message &message, uint64_t deliveryTag, bool redelivered) {

        // install a monitor to deal with the case that the channel is removed
        Monitor monitor(_channel);

        // call the callbacks
        if (_messageCallback) _messageCallback(message, deliveryTag, redelivered);
        
        // we can remove the callback now from the channel
        if (monitor.valid()) _channel->uninstall("");
    });
    
    // return next object
    return _next;
}

/**
 *  Report success, although no message could be get
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
 *  End of namespace
 */
}

