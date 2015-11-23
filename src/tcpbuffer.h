/**
 *  TcpBuffer.h
 *
 *  When data could not be sent out immediately, it is buffered in a temporary
 *  output buffer. This is the implementation of that buffer
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <sys/ioctl.h>
 
/**
 *  Set up namespace
 */
namespace AMQP {
    
/**
 *  Class definition
 */
class TcpBuffer : public Buffer
{
private:
    /**
     *  All output buffers
     *  @var std::deque
     */
    mutable std::deque<std::vector<char>> _buffers;

    /**
     *  Number of bytes in first buffer that is no longer in use
     *  @var size_t
     */
    size_t _skip = 0;
    
    /**
     *  Total number of bytes in the buffer
     *  @var size_t
     */
    size_t _size = 0;

public:
    /**
     *  Regular constructor
     */
    TcpBuffer() {}
    
    /**
     *  No copy'ing allowed
     *  @param  that
     */
    TcpBuffer(const TcpBuffer &that) = delete;

    /**
     *  Move operator
     *  @param  that
     */
    TcpBuffer(TcpBuffer &&that) : 
        _buffers(std::move(that._buffers)), 
        _skip(that._skip), 
        _size(that._size)
    {
        // reset other object
        that._skip = 0;
        that._size = 0;
    }
    
    /**
     *  Move assignment operator
     *  @param  that
     */
    TcpBuffer &operator=(TcpBuffer &&that)
    {
        // skip self-assignment
        if (this == &that) return *this;
        
        // swap buffers
        _buffers.swap(that._buffers);
        
        // swap integers
        std::swap(_skip, that._skip);
        std::swap(_size, that._size);
        
        // done
        return *this;
    }
    
    /**
     *  Does the buffer exist (is it non-empty)
     *  @return bool
     */
    operator bool () const
    {
        // there must be a size
        return _size > 0;
    }
    
    /**
     *  Is the buffer empty
     *  @return bool
     */
    bool operator!() const
    {
        // size should be zero
        return _size == 0;
    }

    /**
     *  Total size of the buffer
     *  @return size_t
     */
    virtual size_t size() const override
    {
        // this simply is a member
        return _size;
    }

    /**
     *  Get access to a single byte
     * 
     *  No safety checks are necessary: this method will only be called
     *  for bytes that actually exist
     * 
     *  @param  pos         position in the buffer
     *  @return char        value of the byte in the buffer
     */
    virtual char byte(size_t pos) const override
    {
        // incorporate the skipped bytes
        pos += _skip;
        
        // iterate over the parts
        for (const auto &buffer : _buffers)
        {
            // is the byte within this buffer?
            if (buffer.size() > pos) return buffer[pos];
            
            // prepare for next iteration
            pos -= buffer.size();
        }
            
        // normally unreachable
        return 0;
    }

    /**
     *  Get access to the raw data
     *  @param  pos         position in the buffer
     *  @param  size        number of continuous bytes
     *  @return char*
     */
    virtual const char *data(size_t pos, size_t size) const override
    {
        // incorporate the skipped bytes
        pos += _skip;
        
        // the buffer into which all data is going to be merged
        std::vector<char> *result = nullptr;
        
        // amount of data that we still have to process
        size_t togo = _size + _skip;
        
        // number of trailing empty buffers
        size_t empty = 0;
        
        // iterate over the parts
        for (auto &buffer : _buffers)
        {
            // are we already merging?
            if (result)
            {
                // merge buffer
                result->insert(result->end(), buffer.begin(), buffer.end());
                
                // one more empty buffer
                ++empty;
            }

            // does the data start within this buffer?
            else if (buffer.size() > pos)
            {
                // remember that this is buffer into which all data is going to be merged
                result = &buffer;
                
                // reserve enough space
                result->reserve(togo);
            }
            
            // data does not start in this part
            else
            {
                // prepare for next iteration
                pos -= buffer.size();
                togo -= buffer.size();
            }
        }
        
        // remove empty buffers
        if (empty > 0) _buffers.resize(_buffers.size() - empty);
        
        // done
        return result->data() + pos;
    }
    
    /**
     *  Copy bytes to a buffer
     * 
     *  No safety checks are necessary: this method will only be called
     *  for bytes that actually exist
     * 
     *  @param  pos         position in the buffer
     *  @param  size        number of bytes to copy
     *  @param  output      buffer to copy into
     *  @return void*       pointer to buffer
     */
    virtual void *copy(size_t pos, size_t size, void *output) const override
    {
        // incorporate the skipped bytes
        pos += _skip;
        
        // number of bytes already copied
        size_t copied = 0;
        
        // iterate over the parts
        for (const auto &buffer : _buffers)
        {
            // is the byte within this buffer?
            if (buffer.size() > pos) 
            {
                // number of bytes to copy 
                size_t tocopy = std::min(buffer.size() - pos, size);
                
                // copy data to the buffer
                memcpy((char *)output + copied, buffer.data() + pos, tocopy);
                
                // update counters
                copied += tocopy;

                // for next iteration we can start on position zero of the next buffer
                pos = 0; size -= tocopy;
                
                // are we alread done?
                if (size == 0) return output;
            }
            else
            {
                // prepare for next iteration
                pos -= buffer.size();
            }
        }
            
        // normally unreachable
        return output;
    }
    
    /**
     *  Add data to the buffer
     *  @param  buffer
     *  @param  size
     */
    void add(const char *buffer, size_t size)
    {
        // add element
        _buffers.emplace_back(buffer, buffer + size);
    
        // update total size
        _size += size;
    }
    
    /**
     *  Shrink the buffer with a number of bytes
     *  @param  toremove
     */
    void shrink(size_t toremove)
    {
        // are we removing everything?
        if (toremove >= _size)
        {
            // reset all
            _buffers.clear(); 
            _skip = _size = 0;
        }
        else
        {
            // keep looping
            while (toremove > 0)
            {
                // access to the first buffer
                const auto &first = _buffers.front();
                
                // actual used bytes in first buffer
                size_t bytes = first.size() - _skip;
                
                // can we remove the first buffer completely?
                if (toremove >= bytes)
                {
                    // we're going to remove the first item, update sizes
                    _size -= bytes;
                    _skip = 0;
                    
                    // number of bytes that still have to be removed
                    toremove -= bytes;
                    
                    // remove first buffer
                    _buffers.pop_front();
                }
                else
                {
                    // we should remove the first buffer partially
                    _skip += toremove;
                    _size -= toremove;
                    
                    // done
                    toremove = 0;
                }
            }
        }
    }
    
    /**
     *  Send the buffer to a socket
     *  @param  socket
     *  @return ssize_t
     */
    ssize_t sendto(int socket)
    {
        // total number of bytes written
        ssize_t total = 0;
        
        // keep looping
        while (_size > 0)
        {
            // we're going to fill a lot of buffers (64 should normally be enough)
            struct iovec buffer[64];
            
            // index counter
            size_t index = 0;
            
            // iterate over the buffers
            for (const auto &str : _buffers)
            {
                // fill buffer
                buffer[index].iov_base = (void *)(index == 0 ? str.data() + _skip : str.data());
                buffer[index].iov_len = index == 0 ? str.size() - _skip : str.size();
                
                // update counter for next iteration
                if (++index >= 64) break;
            }
            
            // send the data
            auto result = writev(socket, (const struct iovec *)&buffer, index);

            // skip on error, or when nothing was written
            if (result <= 0) return total > 0 ? total : result;
            
            // shrink the buffer
            shrink(result);
            
            // update total number of bytes written
            total += 0;
        }
        
        // done
        return total;
    }
    
    /**
     *  Receive data from a socket
     *  @param  socket
     *  @return ssize_t
     */
    ssize_t receivefrom(int socket)
    {
        // find out how many bytes are available
        int available = 0;
        
        // check the number of bytes that are available - in case of an error or 
        // when the buffer is very small, we use a lower limit of 512 bytes
        if (ioctl(socket, FIONREAD, &available) != 0) return -1;

        // if no bytes are available, it could mean that the connection was closed
        // by the remote client, so we do have to call read() anyway, assume a default buffer
        if (available == 0) available = 1;
        
        // add a new buffer
        _buffers.emplace_back(available);
        
        // read the actual buffer
        auto &buffer = _buffers.back();
            
        // read data into the buffer
        auto result = read(socket, buffer.data(), available);
        
        // update total buffer size
        if (result > 0) _size += result;
        
        // if buffer is not full
        if (result < available) buffer.resize(std::max(0L, result));
        
        // done
        return result;
    }
};
    
/**
 *  End of namespace
 */
}

