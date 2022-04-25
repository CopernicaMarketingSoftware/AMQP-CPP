/**
 *  InBuffer.cpp
 *
 *  Implementation of the InBuffer class
 *
 *  @copyright 2014 - 2020 Copernica BV
 */
#include "includes.h"
#include "buffercheck.h"

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Read the next uint8 from the buffer
 *  @param  char* buffer    buffer to read from
 *  @return uint8_t         value read
 */
uint8_t InBuffer::nextUint8()
{
    // check if there is enough size
    BufferCheck check(this, 1);
    
    // get a byte
    return _buffer.byte(_skip);
}

/**
 *  Read the next int8 from the buffer
 *  @param  char* buffer    buffer to read from
 *  @return int8_t          value read
 */
int8_t InBuffer::nextInt8()
{
    // check if there is enough size
    BufferCheck check(this, 1);
    
    // get a byte
    return (int8_t)_buffer.byte(_skip);
}

/**
 *  Read the next uint16_t from the buffer
 *  @return uint16_t        value read
 */
uint16_t InBuffer::nextUint16()
{
    // check if there is enough size
    BufferCheck check(this, sizeof(uint16_t));
    
    // get two bytes, and convert to host-byte-order
    uint16_t value;
    _buffer.copy(_skip, sizeof(uint16_t), &value);
    return be16toh(value);
}

/**
 *  Read the next int16_t from the buffer
 *  @return int16_t     value read
 */
int16_t InBuffer::nextInt16()
{
    // check if there is enough size
    BufferCheck check(this, sizeof(int16_t));
    
    // get two bytes, and convert to host-byte-order
    int16_t value;
    _buffer.copy(_skip, sizeof(int16_t), &value);
    return be16toh(value);
}

/**
 *  Read the next uint32_t from the buffer
 *  @return uint32_t        value read
 */
uint32_t InBuffer::nextUint32()
{
    // check if there is enough size
    BufferCheck check(this, sizeof(uint32_t));
    
    // get four bytes, and convert to host-byte-order
    uint32_t value;
    _buffer.copy(_skip, sizeof(uint32_t), &value);
    return be32toh(value);
}

/**
 *  Read the next int32_t from the buffer
 *  @return uint32_t        value read
 */
int32_t InBuffer::nextInt32()
{
    // check if there is enough size
    BufferCheck check(this, sizeof(int32_t));
    
    // get four bytes, and convert to host-byte-order
    int32_t value;
    _buffer.copy(_skip, sizeof(int32_t), &value);
    return be32toh(value);
}

/**
 *  Read the next uint64_t from the buffer
 *  @return uint64_t        value read
 */
uint64_t InBuffer::nextUint64()
{
    // check if there is enough size
    BufferCheck check(this, sizeof(uint64_t));
    
    // get eight bytes, and convert to host-byte-order
    uint64_t value;
    _buffer.copy(_skip, sizeof(uint64_t), &value);
    return be64toh(value);
}

/**
 *  Read the next uint64_t from the buffer
 *  @return uint64_t        value read
 */
int64_t InBuffer::nextInt64()
{
    // check if there is enough size
    BufferCheck check(this, sizeof(int64_t));
    
    // get eight bytes, and convert to host-byte-order
    int64_t value;
    _buffer.copy(_skip, sizeof(int64_t), &value);
    return be64toh(value);
}

/**
 *  Read a float from the buffer
 *  @return float       float read from buffer. 
 */
float InBuffer::nextFloat()
{
    // check if there is enough size
    BufferCheck check(this, sizeof(float));
    
    // get four bytes
    float value;
    _buffer.copy(_skip, sizeof(float), &value);
    return value;
}

/**
 *  Read a double from the buffer
 *  @return double      double read from buffer
 */
double InBuffer::nextDouble()
{
    // check if there is enough size
    BufferCheck check(this, sizeof(double));
    
    // get eight bytes, and convert to host-byte-order
    double value;
    _buffer.copy(_skip, sizeof(double), &value);
    return value;
}

/**
 *  Get a pointer to the next binary buffer of a certain size
 *  @param  size
 *  @return char*
 */
const char *InBuffer::nextData(size_t size)
{
    // check if there is enough size
    BufferCheck check(this, size);
    
    // get the data
    return _buffer.data(_skip, size);
}

/**
 *  End of namespace
 */
}

