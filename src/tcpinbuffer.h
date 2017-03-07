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
public:
    /**
     *  Constructor
     *  Note that we pass 0 to the constructor because the buffer seems to be empty
     *  @param  size        initial size to allocated
     */
    TcpInBuffer(size_t size) : ByteBuffer((char *)malloc(size), 0) {}
    
    /**
     *  No copy'ing
     *  @param  that        object to copy
     */
    TcpInBuffer(const TcpInBuffer &that) = delete;
    
    /**
     *  Move constructor
     *  @param  that
     */
    TcpInBuffer(TcpInBuffer &&that) : ByteBuffer(std::move(that)) {}
    
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
        
        // call base
        ByteBuffer::operator=(std::move(that));
        
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
   }
    
    /**
     *  Receive data from a socket
     *  @param  socket          socket to read from
     *  @param  expected        number of bytes that the library expects
     *  @return ssize_t
     */
    ssize_t receivefrom(tcp::Socket socket, uint32_t expected)
    {
        // find out how many bytes are available       
        // check the number of bytes that are available
#if _MSC_VER
        u_long available = 0;
        if (ioctlsocket(socket, FIONREAD, &available) != 0) return -1;
#else
        uint32_t available = 0;
        if (ioctl(socket, FIONREAD, &available) != 0) return -1;
#endif

        // if no bytes are available, it could mean that the connection was closed
        // by the remote client, so we do have to call read() anyway, assume a default buffer
        if (available == 0) available = 1;
        
        // number of bytes to read
        uint32_t bytes = std::min((uint32_t)(expected - _size), (uint32_t) available);

        // read data into the buffer
        auto result = recv(socket, const_cast<char *>(_data + _size), bytes, 0);
        
        // update total buffer size
        if (result > 0) _size += result;
        
        // done
        return result;
    }
    
    /**
     *  Shrink the buffer (in practice this is always called with the full buffer size)
     *  @param  size
     */
    void shrink(size_t size)
    {
        // update size
        _size -= size;
    }
};

/**
 *  End of namespace
 */
}

