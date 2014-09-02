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

    // we now know the name, so we can install the message callback on the channel, the self
    // pointer is also captured, which ensures that 'this' is not destructed, all members stay
    // accessible, and that the onFinalize() function will only be called after the message
    // is reported (onFinalize() is called from the destructor of this DeferredGet object)
    _channel->install("", [self, this](const Message &message, uint64_t deliveryTag, bool redelivered) {

        // install a monitor to deal with the case that the channel is removed
        Monitor monitor(_channel);

        // call the callbacks
        if (_messageCallback) _messageCallback(message, deliveryTag, redelivered);
        
        // we can remove the callback now from the channel
        if (monitor.valid()) _channel->uninstall("");
    });

    // report the size (note that this is the size _minus_ the message that is retrieved
    // (and for which the callback will be called later), so it could be zero)
    if (_sizeCallback) _sizeCallback(messageCount);
    
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

