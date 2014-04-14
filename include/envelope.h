#pragma once
/**
 *  Envelope.h
 *
 *  When you send or receive a message to the rabbitMQ server, it is encapsulated
 *  in an envelope that contains additional meta information as well.
 *
 *  @copyright 2014 Copernica BV
 */

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
    Envelope(const char *body, uint64_t size) : MetaData(), _str(body, size) {}

    /**
     *  Constructor based on a string
     *  @param  body
     */
    Envelope(const std::string &body) : MetaData(), _str(body) {}

    /**
     *  Destructor
     */
    virtual ~Envelope() {}

    /**
     *  Access to the full message data
     *  @return buffer
     */
    const char *body() const
    {
        return _str.c_str();
    }

    /**
     *  Size of the body
     *  @return uint64_t
     */
    uint64_t bodySize() const
    {
        return _str.size();
    }

    /**
     *  Body as a string
     *  @return string
     */
    std::string message() const
    {
        return _str;
    }
};

/**
 *  End of namespace
 */
}

