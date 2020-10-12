/**
 *  Confirmed.cpp
 *  
 *  Implementation for Confirmed class.
 *  
 *  @author Michael van der Werve <michael.vanderwerve@mailerq.com>
 *  @copyright 2020 Copernica BV
 */

/**
 *  Includes
 */
#include "includes.h"

/**
 *  Begin of namespaces
 */
namespace AMQP { 

/**
 *  Called when the deliverytag(s) are acked
 *  @param  deliveryTag
 *  @param  multiple
 */
void Confirmed::onAck(uint64_t deliveryTag, bool multiple) 
{
    // monitor the object, watching for destruction since these ack/nack handlers
    // could destruct the object
    Monitor monitor(this);

    // single element is simple
    if (!multiple)
    {
        // find the element
        auto iter = _handlers.find(deliveryTag);

        // we did not find it (this should not be possible, unless somebody explicitly called)
        // the base-class publish methods for some reason.
        if (iter == _handlers.end()) return;

        // call the ack handler
        iter->second->reportAck();

        // if the monitor is no longer valid, we stop (we're done)
        if (!monitor) return;

        // erase it from the map
        _handlers.erase(iter);

        // we are done
        return;
    }

    // find the last element, inclusive
    auto upper = _handlers.upper_bound(deliveryTag);

    // call the handlers
    for (auto iter = _handlers.begin(); iter != upper; iter++)
    {
        // call the handler
        iter->second->reportAck();

        // if we were destructed in the meantime, we leap out
        if (!monitor) return;
    }

    // erase all acknowledged items
    _handlers.erase(_handlers.begin(), upper);

    // call base handler, will advance on the throttle if needed. we call this _after_ we're
    // done processing the callbacks, since one of the callbacks might close the channel, or publish
    // more stuff. additionally, if it does destroy the channel, we are doing a lot of extra publishing
    // for nothing. also, we call some extra handlers, and otherwise we might get onAcked after onClosed
    Throttle::onAck(deliveryTag, multiple);
}

/**
 *  Called when the deliverytag(s) are nacked
 *  @param  deliveryTag
 *  @param  multiple
 */
void Confirmed::onNack(uint64_t deliveryTag, bool multiple)
{
    // monitor the object, watching for destruction since these ack/nack handlers
    // could destruct the object
    Monitor monitor(this);

    // single element is simple
    if (!multiple)
    {
        // find the element
        auto iter = _handlers.find(deliveryTag);

        // we did not find it (this should not be possible, unless somebody explicitly called)
        // the base-class publish methods for some reason.
        if (iter == _handlers.end()) return;

        // call the ack handler
        iter->second->reportNack();

        // if the monitor is no longer valid, we stop (we're done)
        if (!monitor) return;

        // erase it from the map
        _handlers.erase(iter);

        // we are done
        return;
    }

    // find the last element, inclusive
    auto upper = _handlers.upper_bound(deliveryTag);

    // call the handlers
    for (auto iter = _handlers.begin(); iter != upper; iter++)
    {
        // call the handler
        iter->second->reportNack();

        // if we were destructed in the meantime, we leap out
        if (!monitor) return;
    }

    // erase all acknowledged items
    _handlers.erase(_handlers.begin(), upper);

    // call base handler, will advance on the throttle if needed. we call this _after_ we're
    // done processing the callbacks, since one of the callbacks might close the channel, or publish
    // more stuff. additionally, if it does destroy the channel, we are doing a lot of extra publishing
    // for nothing. also, we call some extra handlers, and otherwise we might get onAcked after onClosed
    Throttle::onNack(deliveryTag, multiple);
}

/**
 *  Method that is called to report an error
 *  @param  message
 */
void Confirmed::reportError(const char *message)
{
    // monitor the object, watching for destruction since these ack/nack handlers
    // could destruct the object
    Monitor monitor(this);

    // move the handlers out
    auto handlers = std::move(_handlers);

    // iterate over all the messages
    // call the handlers
    for (const auto &iter : handlers)
    {
        // call the handler
        iter.second->reportError(message);

        // if we were destructed in the meantime, we leap out
        if (!monitor) return;
    }

    // if the monitor is no longer valid, leap out
    if (!monitor) return;
    
    // call base class to let it handle the errors
    Throttle::reportError(message);
}

/**
 *  Publish a message to an exchange. See amqpcpp/channel.h for more details on the flags. 
 *  Delays actual publishing depending on the publisher confirms sent by RabbitMQ.
 * 
 *  @param  exchange    the exchange to publish to
 *  @param  routingkey  the routing key
 *  @param  envelope    the full envelope to send
 *  @param  message     the message to send
 *  @param  size        size of the message
 *  @param  flags       optional flags
 */
DeferredConfirmedPublish &Confirmed::publish(const std::string &exchange, const std::string &routingKey, const Envelope &envelope, int flags)
{
    // copy the current identifier, this will be the ID that will come back
    auto current = _current;

    // publish the entire thing, and remember if it failed at any point
    bool failed = !Throttle::publish(exchange, routingKey, envelope, flags);
    
    // create the open
    auto handler = std::make_shared<DeferredConfirmedPublish>(failed);

    // add it to the open handlers
    _handlers[current] = handler;

    // return the dereferenced handler 
    return *handler;
}

/**
 *  End of namespaces
 */
} 
