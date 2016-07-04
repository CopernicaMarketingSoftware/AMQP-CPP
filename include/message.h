/**
 *  Message.h
 *
 *  An incoming message has the same sort of information as an outgoing
 *  message, plus some additional information.
 *
 *  Message objects can not be constructed by end users, they are only constructed
 *  by the AMQP library, and passed to user callbacks.
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include "envelope.h"
#include <limits>
#include <stdexcept>

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Forward declarations
 */
class DeferredConsumerBase;

/**
 *  Class definition
 */
class Message : public Envelope
{
protected:
    /**
     *  The exchange to which it was originally published
     *  @var    string
     */
    std::string _exchange;

    /**
     *  The routing key that was originally used
     *  @var    string
     */
    std::string _routingKey;

    /**
     *  We are an open book to the consumer handler
     */
    friend class DeferredConsumerBase;

    /**
     *  Set the body size
     *  This field is set when the header is received
     *  @param  uint64_t
     */
    void setBodySize(uint64_t size)
    {
        // safety-check: on 32-bit platforms size_t is obviously also a 32-bit dword
        // in which case casting the uint64_t to a size_t could result in truncation
        // here we check whether the given size fits inside a size_t
        if (std::numeric_limits<size_t>::max() < size) throw std::runtime_error("message is too big for this system");

        // store the new size
        _bodySize = size;
    }

    /**
     *  Append data
     *  @param  buffer      incoming data
     *  @param  size        size of the data
     *  @return bool        true if the message is now complete
     */
    bool append(const char *buffer, uint64_t size)
    {
        // is this the only data, and also direct complete?
        if (_str.empty() && size >= _bodySize)
        {
            // we have everything
            _body = buffer;

            // done
            return true;
        }
        else
        {
            // it does not fit yet, do we have to allocate
            if (!_body)
            {
                // allocate memory in the string
                _str.reserve(static_cast<size_t>(_bodySize));

                // we now use the data buffer inside the string
                _body = _str.data();
            }

            // safety-check: if the given size exceeds the given message body size
            // we truncate it, this should never happen because it indicates a bug
            // in the AMQP server implementation, should we report this?
            size = std::min(size, _bodySize - _str.size());

            // we can not safely append the data to the string, it
            // will not exceed the reserved size so it is guaranteed
            // not to change the data pointer, we can just leave that
            // @todo this is not always necessary; instead, we can refrain from
            // allocating this buffer entirely and just insert it into the message
            // directly.
            _str.append(buffer, static_cast<size_t>(size));

            // if the string is filled with the given number of characters we are done now
            return _str.size() >= _bodySize;
        }
    }

public:
    /**
     *  Constructor
     *
     *  @param  exchange
     *  @param  routingKey
     */
    Message(const std::string &exchange, const std::string &routingKey) :
        Envelope(nullptr, 0), _exchange(exchange), _routingKey(routingKey)
    {}


    /**
     *  Copy constructor
     *
     *  @param  message the message to copy
     */
    Message(const Message &message) :
        Envelope(message),
        _exchange(message._exchange),
        _routingKey(message._routingKey)
    {}

    /**
     *  Move constructor
     *
     *  @param  message the message to move
     */
    Message(Message &&message) :
        Envelope(std::move(message)),
        _exchange(std::move(message._exchange)),
        _routingKey(std::move(message._routingKey))
    {}

    /**
     *  Destructor
     */
    virtual ~Message() = default;

    /**
     *  Assignment operator
     *
     *  @param  message the message to copy
     *  @return same object for chaining
     */
    Message &operator=(const Message &message)
    {
        // call the base assignment
        Envelope::operator=(message);

        // move the exchange and routing key
        _exchange   = message._exchange;
        _routingKey = message._routingKey;

        // allow chaining
        return *this;
    }

    /**
     *  Move assignment operator
     *
     *  @param  message the message to move
     *  @return same object for chaining
     */
    Message &operator=(Message &&message)
    {
        // call the base assignment
        Envelope::operator=(std::move(message));

        // move the exchange and routing key
        _exchange   = std::move(message._exchange);
        _routingKey = std::move(message._routingKey);

        // allow chaining
        return *this;
    }

    /**
     *  The exchange to which it was originally published
     *  @var    string
     */
    const std::string &exchange() const
    {
        return _exchange;
    }

    /**
     *  The routing key that was originally used
     *  @var    string
     */
    const std::string &routingKey() const
    {
        return _routingKey;
    }
};

/**
 *  End of namespace
 */
}

