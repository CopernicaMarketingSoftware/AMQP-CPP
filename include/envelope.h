/**
 *  Envelope.h
 *
 *  When you send or receive a message to the rabbitMQ server, it is encapsulated
 *  in an envelope that contains additional meta information as well.
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
#include "metadata.h"

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class Envelope : public MetaData
{
protected:
    /**
     *  The body (only used when string object was passed to constructor
     *  @var    std::string
     */
    std::string _str;

    /**
     *  Pointer to the body data (the memory buffer is not managed by the AMQP
     *  library!)
     *  @var    const char *
     */
    const char *_body;

    /**
     *  Size of the data
     *  @var    uint64_t
     */
    uint64_t _bodySize;

public:
    /**
     *  Constructor
     *
     *  The data buffer that you pass to this constructor must be valid during
     *  the lifetime of the Envelope object.
     *
     *  @param  body
     *  @param  size
     */
    Envelope(const char *body, uint64_t size) : MetaData(), _body(body), _bodySize(size) {}

    /**
     *  Constructor based on a string
     *  @param  body
     */
    Envelope(const std::string &body) : MetaData(), _str(body), _body(_str.data()), _bodySize(_str.size()) {}

    /**
     *  Constructor based on a string
     *  @param  body
     */
    Envelope(std::string &&body) : MetaData(), _str(std::move(body)), _body(_str.data()), _bodySize(_str.size()) {}

    /**
     *  Copy constructor
     *
     *  @param  envelope    the envelope to copy
     */
    Envelope(const Envelope &envelope) :
        MetaData(envelope),
        _str(envelope._body, envelope._bodySize),
        _body(_str.data()),
        _bodySize(_str.size())
    {}

    /**
     *  Move constructor
     *
     *  @param  envelope    the envelope to move
     */
    Envelope(Envelope &&envelope) :
        MetaData(std::move(envelope)),
        _str(std::move(envelope._str)),
        _body(_str.data()),
        _bodySize(_str.size())
    {
        // if the envelope we moved did not have allocation by string
        // we are out of luck, and have to copy it ourselves :(
        if (_str.empty())
        {
            // assign the data from the other envelope
            _str.assign(envelope._body, envelope._bodySize);

            // and set the correct pointer and size
            _body = _str.data();
            _bodySize = _str.size();
        }
        else
        {
            // we moved the other envelopes string
            // which means their body pointer is now
            // garbage (it no longer points to a valid
            // address), so we need to clear it
            envelope._body = nullptr;
            envelope._bodySize = 0;
        }
    }

    /**
     *  Destructor
     */
    virtual ~Envelope() {}

    /**
     *  Assignment operator
     *
     *  @param  envelope    the envelope to copy
     *  @return same object for chaining
     */
    Envelope &operator=(const Envelope &envelope)
    {
        // copy the data from the envelope
        _str.assign(envelope._body, envelope._bodySize);

        // set the data pointer and body size
        _body = _str.data();
        _bodySize = _str.size();

        // allow chaining
        return *this;
    }

    /**
     *  Move assignment operator
     *
     *  @param  envelope    the envelope to move
     *  @return same object for chaining
     */
    Envelope &operator=(Envelope &&envelope)
    {
        // was the string in the other envelop empty?
        if (envelope._str.empty())
        {
            // that's a shame, we have to make a full copy
            _str.assign(envelope._body, envelope._bodySize);
        }
        else
        {
            // not empty, just move it
            _str = std::move(envelope._str);

            // their string is now garbage so the
            // pointer is also invalid
            envelope._body = nullptr;
            envelope._bodySize = 0;
        }

        // we now have a valid string, set the body and
        _body = _str.data();
        _bodySize = _str.size();

        // allow chaining
        return *this;
    }

    /**
     *  Access to the full message data
     *  @return buffer
     */
    const char *body() const
    {
        return _body;
    }

    /**
     *  Size of the body
     *  @return uint64_t
     */
    uint64_t bodySize() const
    {
        return _bodySize;
    }

    /**
     *  Body as a string
     *  @return string
     */
    std::string message() const
    {
        return std::string(_body, static_cast<size_t>(_bodySize));
    }
};

/**
 *  End of namespace
 */
}

