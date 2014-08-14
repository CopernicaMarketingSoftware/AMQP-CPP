/**
 *  ByteByffer.h
 *
 *  Very simple implementation of the buffer class that simply wraps
 *  around a buffer of bytes
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2014 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Open namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class ByteBuffer : public Buffer
{
private:
    /**
     *  The actual byte buffer
     *  @var const char *
     */
//    const char *_data;
    
    /**
     *  Size of the buffer
     *  @var size_t
     */
//    size_t _size;

public:
    /**
     *  Constructor
     *  @param  data
     *  @param  size
     */
    ByteBuffer(const char *data, size_t size) //: _data(data), _size(size)
    {}
    
    /**
     *  Destructor
     */
    virtual ~ByteBuffer() {}

};

/**
 *  End namespace
 */
}
