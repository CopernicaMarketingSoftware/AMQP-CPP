/**
 *  Throttle.cpp
 *  
 *  Implementation for Throttle class.
 *  
 *  @author Michael van der Werve <michael.vanderwerve@mailerq.com>
 *  @copyright 2020 Copernica BV
 */

/**
 *  Includes
 */
#include "includes.h"
#include "basicpublishframe.h"
#include "basicheaderframe.h"
#include "bodyframe.h"
#include <iostream>

/**
 *  Begin of namespaces
 */
namespace AMQP { 

/**
 *  Constructor
 *  @param  channel 
 *  @param  throttle
 */
Throttle::Throttle(Channel &channel, size_t throttle) : Tagger(channel), _throttle(throttle) {}

/**
 *  Called when the deliverytag(s) are acked
 *  @param  deliveryTag
 *  @param  multiple
 */
void Throttle::onAck(uint64_t deliveryTag, bool multiple)
{
    // number of messages exposed
    if (multiple) _open.erase(_open.begin(), _open.upper_bound(deliveryTag));

    // otherwise, we remove the single element
    else _open.erase(deliveryTag);

    // if there is room, flush part of the queue
    if (_open.size() < _throttle) flush(_throttle - _open.size());

    // call base handler
    Tagger::onAck(deliveryTag, multiple);
}

/**
 *  Called when the deliverytag(s) are nacked
 *  @param  deliveryTag
 *  @param  multiple
 */
void Throttle::onNack(uint64_t deliveryTag, bool multiple)
{
    // number of messages exposed
    if (multiple) _open.erase(_open.begin(), _open.upper_bound(deliveryTag));

    // otherwise, we remove the single element
    else _open.erase(deliveryTag);

    // if there is room, flush part of the queue
    if (_open.size() < _throttle) flush(_throttle - _open.size());

    // call base handler
    Tagger::onNack(deliveryTag, multiple);
}

/**
 *  Send method for a frame
 *  @param  id
 *  @param  frame
 */
bool Throttle::send(uint64_t id, const Frame &frame)
{
    // if there is already a queue, we always append it
    if (!_queue.empty() || (_open.size() >= _throttle && _last != id))
    {
        // add the element to the queue
        _queue.emplace(id, frame);

        // we have successfully added the message
        return true;
    }

    // there is no queue and we have space, we send it directly
    _last = id;

    // we have now send this id
    _open.insert(id);

    // we can finally actually send it
    return Tagger::send(id, frame);
}

/**
 *  Method that is called to report an error
 *  @param  message
 */
void Throttle::reportError(const char *message)
{
    // pop all elements from the queue (older compilers dont support reassign)
    while (_queue.size()) _queue.pop();

    // we can also forget all open messages, won't hear from them any more
    _open.clear();

    // we have no last seen message any more
    _last = 0;

    // call base method
    Tagger::reportError(message);
}

/**
 *  Flush the throttle
 *  @param  max
 */
size_t Throttle::flush(size_t max)
{
    // how many have we published
    size_t published = 0;

    // keep sending more messages while there is a queue
    while (!_queue.empty())
    {
        // get the front element from the queue
        auto &front = _queue.front();

        // if the front has a different tag, we might not be allowed to continue
        if (front.first != _last)
        {
            // this is an extra publish, check if this puts us over the edge, in which case we 
            // did one less (unless max = 0, which means do a full flush)
            if (max > 0 && published >= max) return published;

            // we are going to publish an extra message
            ++published;

            // we now go to publish a new element
            _last = front.first;

            // insert it into the set as well
            _open.insert(_last);
        }

        // send the buffer over the implementation
        _implementation->send(std::move(front.second));

        // and remove the message
        _queue.pop();
    }

    // return number of published messages.
    return published;
}

/**
 *  End of namespaces
 */
} 
