/**
 *  Message.h
 *
 *  An incoming message has the same sort of information as an outgoing
 *  message, plus some additional information.
 *
 *  Message objects can not be constructed by end users, they are only constructed
 *  by the AMQP library, and passed to user callbacks.
 *
 *  @copyright 2014 - 2017 Copernica BV
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
#include <algorithm>

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
    std::string _routingkey;
    
    /**
     *  Number of bytes already filled
     *  @var    size_t
     */
    size_t _filled = 0;

    
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
        // is the body already allocated?
        if (_allocated)
        {
            // prevent overflow
            size = std::min(size, _bodySize - _filled);
            
            // append more data
            memcpy(_body + _filled, buffer, size);
            
            // update filled data
            _filled += size;
        }
        else if (size >= _bodySize)
        {
            // we do not have to combine multiple frames, so we can store
            // the buffer pointer in the message 
            _body = (char *)buffer;
        }
        else
        {
            // allocate the buffer
            _body = (char *)malloc(_bodySize);
            
            // remember that the buffer was allocated, so that the destructor can get rid of it
            _allocated = true;
            
            // append more data
            memcpy(_body, buffer, std::min(size, _bodySize));
            
            // update filled data
            _filled = std::min(size, _bodySize);
        }
            
        // check if we're done
        return _filled >= _bodySize;
    }

public:
    /**
     *  Constructor
     *
     *  @param  exchange
     *  @param  routingKey
     */
    Message(std::string exchange, std::string routingkey) :
        Envelope(nullptr, 0), _exchange(std::move(exchange)), _routingkey(std::move(routingkey))
    {}

    /**
     *  Disabled copy constructor
     *  @param  message the message to copy
     */
    Message(const Message &message) = delete;

    /**
     *  Destructor
     */
    virtual ~Message() = default;

    /**
     *  The exchange to which it was originally published
     *  @var    string
     */
    const std::string &exchange() const
    {
        // expose member
        return _exchange;
    }

    /**
     *  The routing key that was originally used
     *  @var    string
     */
    const std::string &routingkey() const
    {
        // expose member
        return _routingkey;
    }
};

/**
 *  End of namespace
 */
}

