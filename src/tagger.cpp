/**
 *  Tagger.cpp
 *  
 *  Implementation for Tagger class.
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

/**
 *  Begin of namespaces
 */
namespace AMQP { 

/**
 *  Constructor
 *  @param  channel 
 */
Tagger::Tagger(Channel &channel) : _implementation(channel._implementation)
{
    // activate confirm-select mode
    auto &deferred = channel.confirmSelect()
        .onAck([this](uint64_t deliveryTag, bool multiple) { onAck(deliveryTag, multiple); })
        .onNack([this](uint64_t deliveryTag, bool multiple, bool /* requeue*/) { onNack(deliveryTag, multiple); });

    // we might have failed, in which case we throw
    if (!deferred) throw std::runtime_error("could not enable publisher confirms");

    // we wrap a handling error callback that calls our member function
    _implementation->onError([this](const char *message) { reportError(message); });  
}

/**
 *  Destructor
 */
Tagger::~Tagger()
{
    // restore the error-callback
    _implementation->onError(nullptr);
    
    // also unset the callbacks for onAck and onNack
    auto *deferred = _implementation->confirm();
    
    // unlikely case that the onAck and onNack are not set
    if (deferred == nullptr) return;
    
    // unset the callbacks
    deferred->onAck(nullptr).onNack(nullptr);
}

/**
 *  Send method for a frame
 *  @param  id
 *  @param  frame
 */
bool Tagger::send(uint64_t id, const Frame &frame)
{
    // we're simply going to send it over the channel directly
    return _implementation->send(frame);
}

/**
 *  Called when the deliverytag(s) are acked
 *  @param  deliveryTag
 *  @param  multiple
 */
void Tagger::onAck(uint64_t deliveryTag, bool multiple)
{
    // leap out if there are still messages or we shouldn't close yet
    if (!_close || unacknowledged()) return;

    // close the channel, and forward the callbacks to the installed handler
    // we need to be sure the the deffered object stays alive even if the callback
    // decides to remove us.
    _implementation->close()
        .onSuccess([this]() { auto close = _close; close->reportSuccess(); })
        .onError([this](const char *message) { auto close = _close; close->reportError(message); });
}

/**
 *  Called when the deliverytag(s) are nacked
 *  @param  deliveryTag
 *  @param  multiple
 */
void Tagger::onNack(uint64_t deliveryTag, bool multiple)
{
    // leap out if there are still messages or we shouldn't close yet
    if (!_close || unacknowledged()) return;

    // close the channel, and forward the callbacks to the installed handler
    // we need to be sure the the deffered object stays alive even if the callback
    // decides to remove us.
    _implementation->close()
        .onSuccess([this]() { auto close = _close; close->reportSuccess(); })
        .onError([this](const char *message) { auto close = _close; close->reportError(message); });
}

/**
 *  Method that is called to report an error
 *  @param  message
 */
void Tagger::reportError(const char *message)
{
    // reset tracking, since channel is fully broken
    _current = 1;

    // if a callback is set, call the handler with the message
    if (_errorCallback) _errorCallback(message);
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
 *  @return uint64_t
 */
uint64_t Tagger::publish(const std::string &exchange, const std::string &routingKey, const Envelope &envelope, int flags)
{
    // @todo do not copy the entire buffer to individual frames

    // fail if we're closing the channel, no more publishes allowed
    if (_close) return false;
    
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

    // we succeeded
    return _current++;
}

/**
 *  Close the throttle channel (closes the underlying channel)
 *  @return Deferred&
 */
Deferred &Tagger::close()
{
    // if this was already set to be closed, return that
    if (_close) return *_close;

    // create the deferred
    _close = std::make_shared<Deferred>(!_implementation->usable());

    // if there are open messages or there is a queue, they will still get acked and we will then forward it
    if (unacknowledged()) return *_close;

    // there are no open messages, we can close the channel directly.
    // we need to be sure the the deffered object stays alive even if the callback
    // decides to remove us.
    _implementation->close()
        .onSuccess([this]() { auto close = _close; close->reportSuccess(); })
        .onError([this](const char *message) { auto close = _close; close->reportError(message); });

    // return the created deferred
    return *_close;
}

/**
 *  Install an error callback
 *  @param  callback
 */
void Tagger::onError(ErrorCallback&& callback)
{
    // we store the callback
    _errorCallback = std::move(callback);

    // check the callback
    if (!_errorCallback) return;

    // if the channel is no longer usable, report that
    if (!_implementation->usable()) return _errorCallback("Channel is no longer usable");

    // specify that we're already closing
    if (_close) _errorCallback("Wrapped channel is closing down");
}

/**
 *  End of namespaces
 */
} 
