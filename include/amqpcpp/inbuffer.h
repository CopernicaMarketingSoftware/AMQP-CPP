/**
 *  InBuffer.h
 *
 *  The InBuffer class is a wrapper around a data buffer and that adds
 *  some safety checks so that the rest of the library can safely read
 *  from it.
 *
 *  This is a class that is used internally by the AMQP library. As a user
 *  of this library, you normally do not have to instantiate it. However,
 *  if you do want to store or safe messages yourself, it sometimes can
 *  be useful to implement it.
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
#include <cstdint>

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Forward declarations
 */
class Buffer;

/**
 *  Class definition
 */
class InBuffer
{
protected:
    /**
     *  The buffer we are reading from
     *  @var    Buffer
     */
    const Buffer &_buffer;

    /**
     *  Number of bytes already processed
     *  @var    size_t
     */
    size_t _skip = 0;

    template <typename T, std::enable_if_t<(sizeof(T) == 1) || (static_cast<uint8_t>(0x1234) == 0x12), bool> = true>
    const T& be_to_h(const T& value)
    {
        return value;
    }

    template <typename T, std::enable_if_t<(sizeof(T) > 1) && (static_cast<uint8_t>(0x1234) == 0x34), bool> = true>
    T be_to_h(const T& value)
    {
        size_t c = sizeof(T);
        const uint8_t(&i)[sizeof(T)] = reinterpret_cast<const uint8_t(&)[sizeof(T)]> (value);
        uint8_t o[sizeof(T)] = {};
        for (auto& v : i)
            o[--c] = v;
        return reinterpret_cast<const T&>(o);
    }

public:
    /**
     *  Constructor
     *  @param  buffer      Binary buffer
     */
    InBuffer(const Buffer &buffer) : _buffer(buffer) {}

    /**
     *  Destructor
     */
    virtual ~InBuffer() {}

    /**
     *  Read the next uint8_t from the buffer
     *  @return uint8_t         value read
     */
    uint8_t nextUint8();

    /**
     *  Read the next int8_t from the buffer
     *  @return int8_t          value read
     */
    int8_t nextInt8();

    /**
     *  Read the next uint16_t from the buffer
     *  @return uint16_t        value read
     */
    uint16_t nextUint16();

    /**
     *  Read the next int16_t from the buffer
     *  @return int16_t     value read
     */
    int16_t nextInt16();

    /**
     *  Read the next uint32_t from the buffer
     *  @return uint32_t        value read
     */
    uint32_t nextUint32();

    /**
     *  Read the next int32_t from the buffer
     *  @return int32_t     value read
     */
    int32_t nextInt32();

    /**
     *  Read the next uint64_t from the buffer
     *  @return uint64_t        value read
     */
    uint64_t nextUint64();

    /**
     *  Read the next int64_t from the buffer
     *  @return int64_t     value read
     */
    int64_t nextInt64();

    /**
     *  Read a float from the buffer
     *  @return float       float read from buffer.
     */
    float nextFloat();

    /**
     *  Read a double from the buffer
     *  @return double      double read from buffer
     */
    double nextDouble();

    /**
     *  Read a numeric from the buffer
     *  @return double      double read from buffer
     */
    template <typename T, std::enable_if_t<sizeof(T) == 1, bool> = true>
    T nextNumeric()
    {
        auto val = nextUint8();
        return reinterpret_cast<const T&>(val);
    }
    template <typename T, std::enable_if_t<sizeof(T) == 2, bool> = true>
    T nextNumeric()
    {
        auto val = nextUint16();
        return reinterpret_cast<const T&>(val);
    }
    template <typename T, std::enable_if_t<sizeof(T) == 4, bool> = true>
    T nextNumeric()
    {
        auto val = nextUint32();
        return reinterpret_cast<const T&>(val);
    }
    template <typename T, std::enable_if_t<sizeof(T) == 8, bool> = true>
    T nextNumeric()
    {
        auto val = nextUint64();
        return reinterpret_cast<const T&>(val);
    }

    /**
     *  Get a pointer to the next binary buffer of a certain size
     *  @param  size
     *  @return char*
     */
    const char *nextData(size_t size);

    /**
     *  The checker may access private data
     */
    friend class BufferCheck;

};

/**
 *  End of namespace
 */
}
