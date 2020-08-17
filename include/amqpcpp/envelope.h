/**
 *  Envelope.h
 *
 *  When you send or receive a message to the rabbitMQ server, it is encapsulated
 *  in an envelope that contains additional meta information as well.
 *
 *  @copyright 2014 - 2020 Copernica BV
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
 *  The envelope extends from MetaData, although this is conceptually not entirely
 *  correct: and envelope _holds_ meta data and a body, so it would have been more
 *  correct to make the MetaData instance a member. But by extending we automatically
 *  make all meta-data properties accesible.
 */
class Envelope : public MetaData
{
protected:
    /**
     *  Pointer to the body data (the memory is not managed by the AMQP library!)
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
     *  The data buffer that you pass to this constructor must be valid during
     *  the lifetime of the Envelope object.
     *  @param  body
     *  @param  size
     */
    Envelope(const char *body, uint64_t size) : MetaData(), _body(body), _bodySize(size) {}

    /**
     *  Read envelope frmo an input-buffer
     *  This method is the counterpart of the Envelope::fill() method, and is not used
     *  by the library itself, but might be useful for applications that want to store
     *  envelopes.
     *  @param  frame
     */
    Envelope(InBuffer &buffer) : MetaData(buffer)
    {
        // extract the properties
        _bodySize = buffer.nextUint64();
        _body = buffer.nextData(_bodySize);
    }

    /**
     *  Disabled copy constructor
     *  @param  envelope    the envelope to copy
     */
    Envelope(const Envelope &envelope) = delete;

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
     *  Size of the envelope, this is the size of the meta+data plus the number of bytes
     *  required to store the size of the body + the actual body. This method is not used
     *  by the AMQP-CPP library, but could be useful if you feel the need to store 
     *  @return size_t
     */
    size_t size() const
    {
        // this is the size of the meta-data + the size of the body 
        return MetaData::size() + _bodySize + sizeof(uint64_t);
    }
    
    /**
     *  Fill an output buffer
     *  This method is not used by this library, but could be useful if you want to store
     *  the meta-data + message contents (
     *  @param  buffer
     */
    void fill(OutBuffer &buffer) const
    {
        // first we store the meta-data
        MetaData::fill(buffer);
        
        // now the size of the message body + the actual body
        buffer.add(_bodySize);
        buffer.add(_body, _bodySize);
    }
};

/**
 *  End of namespace
 */
}

