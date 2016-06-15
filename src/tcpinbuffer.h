/**
 *  TcpInBuffer.h
 *
 *  Implementation of byte byte-buffer used for incoming frames
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2016 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Beginnig of namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class TcpInBuffer : public ByteBuffer
{
private:
    /**
     *  Number of bytes already filled
     *  @var size_t
     */
    size_t _filled = 0;

public:
    /**
     *  Constructor
     *  @param  size        initial size
     */
    TcpInBuffer(size_t size) : ByteBuffer((char *)malloc(size), size) {}
    
    /**
     *  No copy'ing
     *  @param  that        object to copy
     */
    TcpInBuffer(const TcpInBuffer &that) = delete;
    
    /**
     *  Move constructor
     *  @param  that
     */
    TcpInBuffer(TcpInBuffer &&that) : ByteBuffer(std::move(that)), _filled(that._filled)
    {
        // reset other object
        that._filled = 0;
    }
    
    /**
     *  Destructor
     */
    virtual ~TcpInBuffer()
    {
        // free memory
        if (_data) free((void *)_data);
    }

    /**
     *  Move assignment operator
     *  @param  that
     */
    TcpInBuffer &operator=(TcpInBuffer &&that)
    {
        // skip self-assignment
        if (this == &that) return *this;
        
        // copy the filled paramteer
        _filled = that._filled;
        
        // reset other object
        that._filled = 0;
        
        // call base
        ByteBuffer::operator=(std::move(*this));
        
        // done
        return *this;
    }
    
    /**
     *  Reallocate date
     *  @param  size
     */
    void reallocate(size_t size)
    {
        // update data
        _data = (char *)realloc((void *)_data, size);

        // update size
        _size = size;
    }
    
    /**
     *  Receive data from a socket
     *  @param  socket          socket to read from
     *  @param  expected        number of bytes that the library expects
     *  @return ssize_t
     */
    ssize_t receivefrom(int socket, uint32_t expected)
    {
        // find out how many bytes are available
        uint32_t available = 0;
        
        // check the number of bytes that are available
        if (ioctl(socket, FIONREAD, &available) != 0) return -1;

        // if no bytes are available, it could mean that the connection was closed
        // by the remote client, so we do have to call read() anyway, assume a default buffer
        if (available == 0) available = 1;
        
        // number of bytes to read
        size_t bytes = std::min(expected, available);
        
        // read data into the buffer
        auto result = read(socket, (void *)(_data + _filled), bytes);
        
        // update total buffer size
        if (result > 0) _filled += result;
        
        // done
        return result;
    }
    
    /**
     *  Shrink the buffer (in practice this is always called with the full buffer size)
     *  @param  size
     */
    void shrink(size_t size)
    {
        // update filled bytes
        _filled -= size;
    }
};

/**
 *  End of namespace
 */
}

