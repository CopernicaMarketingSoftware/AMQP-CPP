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
Throttle::Throttle(Channel &channel, size_t throttle) : _implementation(channel._implementation), _throttle(throttle)
{
    // activate confirm-select mode
    auto &deferred = channel.confirmSelect()
        .onAck([this](uint64_t deliveryTag, bool multiple) { onAck(deliveryTag, multiple); })
        .onNack([this](uint64_t deliveryTag, bool multiple, bool /* requeue*/) { onAck(deliveryTag, multiple); });

    // we might have failed, in which case we throw
    if (!deferred) throw std::runtime_error("could not enable publisher confirms");
}

/**
 *  Called when the deliverytag(s) are acked/nacked
 *  @param  deliveryTag
 *  @param  multiple
 */
void Throttle::onAck(uint64_t deliveryTag, bool multiple)
{
    // number of messages exposed
    if (multiple) _open.erase(_open.begin(), _open.upper_bound(deliveryTag));

    // otherwise, we remove the single element
    else _open.erase(deliveryTag);

    // keep sending more messages while there is a queue
    while (!_queue.empty())
    {
        // get the front element from the queue
        // @todo move it to the channel
        auto &front = _queue.front();

        // if the front has a different tag, we might not be allowed to continue
        if (front.first != _last)
        {
            // if there is no more room, we're done, stop
            if (_open.size() >= _throttle) return;

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

    // and we're going to send it over the channel directly
    return _implementation->send(frame);
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
bool Throttle::publish(const std::string &exchange, const std::string &routingKey, const Envelope &envelope, int flags)
{
    // @todo do not copy the entire buffer to individual frames
    
    // send the publish frame
    if (!send(_current, BasicPublishFrame(_implementation->id(), exchange, routingKey, (flags & mandatory) != 0, (flags & immediate) != 0))) return false;

    // send header
    if (!send(_current, BasicHeaderFrame(_implementation->id(), envelope))) return false;

    // connection and channel still usable?
    if (!_implementation->usable()) return false;

    // the max payload size is the max frame size minus the bytes for headers and trailer
    uint32_t maxpayload = _implementation->maxPayload();
    uint64_t bytessent = 0;

    // the buffer
    const char *data = envelope.body();
    uint64_t bytesleft = envelope.bodySize();

    // split up the body in multiple frames depending on the max frame size
    while (bytesleft > 0)
    {
        // size of this chunk
        uint64_t chunksize = std::min(static_cast<uint64_t>(maxpayload), bytesleft);

        // send out a body frame
        if (!send(_current, BodyFrame(_implementation->id(), data + bytessent, (uint32_t)chunksize))) return false;

        // update counters
        bytessent += chunksize;
        bytesleft -= chunksize;
    }

    // we're done, we move to the next deliverytag
    ++_current;

    // we succeeded
    return true;
}

/**
 *  End of namespaces
 */
} 
