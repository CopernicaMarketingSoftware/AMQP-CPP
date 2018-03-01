/**
 *  DeferredPublisher.cpp
 *
 *  Implementation file for the DeferredPublisher class
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2018 Copernica BV
 */
#include "includes.h"

/**
 *  Begin of namespace
 */
namespace AMQP {

/**
 *  Indicate that a message was done
 */
void DeferredPublisher::complete()
{
    // also monitor the channel
    Monitor monitor(_channel);

    // do we have a message?
    if (_message) _bounceCallback(*_message, _code, _description);

    // do we have to inform anyone about completion?
    if (_completeCallback) _completeCallback();
    
    // for the next iteration we want a new message
    _message.reset();

    // do we still have a valid channel
    if (!monitor.valid()) return;

    // we are now done executing, so the channel can forget the current receiving object
    _channel->install(nullptr);
}

/**
 *  End of namespace
 */
}


