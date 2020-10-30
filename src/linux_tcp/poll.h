/**
 *  Poll.h
 *
 *  Class to check or wait for a socket to become readable and/or writable
 *
 *  @copyright 2018 Copernica BV
 */
 
 /**
 *  Include guard
 */
#pragma once

/**
 *  Includes
 */
#include <poll.h>

/**
 *  Begin of namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class Poll
{
private:
    /**
     *  Set with just one filedescriptor
     *  @var fd_set
     */
    pollfd _fd;

    /**
     *  The socket filedescriptor
     *  @var int
     */
    int _socket;
    
public:
    /**
     *  Constructor
     *  @param  fd      the filedescriptor that we're waiting on
     */
    Poll(int fd)
    {
        // set the fd
        _fd.fd = fd;
    }
    
    /**
     *  No copying
     *  @param  that
     */
    Poll(const Poll &that) = delete;
    
    /**
     *  Destructor
     */
    virtual ~Poll() = default;
    
    /**
     *  Check if a file descriptor is readable.
     *  @return bool
     */
    bool readable() 
    {   
        // check for readable
        _fd.events = POLLIN;
        _fd.revents = 0;     

        // poll the fd with no timeout
        return poll(&_fd, 1, 0) > 0;
    }
        
    /**
     *  Wait until the filedescriptor becomes writable
     *  @return bool
     */
    bool writable() 
    {
        // check for readable
        _fd.events = POLLOUT;   
        _fd.revents = 0;  

        // poll the fd with no timeout
        return poll(&_fd, 1, 0) > 0;
    }
    
    /**
     *  Wait until a filedescriptor becomes active (readable or writable)
     *  @return bool
     */
    bool active()
    {
        // check for readable
        _fd.events = POLLIN | POLLOUT;     
        _fd.revents = 0;

        // poll the fd with no timeout
        return poll(&_fd, 1, 0) > 0;
    }
};

/**
 *  End of namespace
 */
}
