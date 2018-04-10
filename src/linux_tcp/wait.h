/**
 *  Wait.h
 *
 *  Class to wait for a socket to become readable and/or writable
 *
 *  @copyright 2018 Copernica BV
 */
 
 /**
 *  Include guard
 */
#pragma once

/**
 *  Begin of namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class Wait
{
private:
    /**
     *  Set with just one filedescriptor
     *  @var fd_set
     */
    fd_set _set;

    /**
     *  The current socket // @todo what is it exactly?
     *  @var int
     */
    int _socket;
    
public:
    /**
     *  Constructor
     *  @param  fd      the filedescriptor that we're waiting on
     */
    Wait(int fd) : _socket(fd)
    {
        // initialize the set
        FD_ZERO(&_set);
        
        // add the one socket
        FD_SET(_socket, &_set);
    }
    
    /**
     *  No copying
     *  @param  that
     */
    Wait(const Wait &that) = delete;
    
    /**
     *  Destructor
     */
    virtual ~Wait() = default;
    
    /**
     *  Wait until the filedescriptor becomes readable
     *  @return bool
     */
    bool readable() 
    {
        // wait for the socket
        return select(_socket + 1, &_set, nullptr, nullptr, nullptr) > 0;
    }
        
    /**
     *  Wait until the filedescriptor becomes writable
     *  @return bool
     */
    bool writable() 
    {
        // wait for the socket
        return select(_socket + 1, nullptr, &_set, nullptr, nullptr) > 0;
    }
    
    /**
     *  Wait until a filedescriptor becomes active (readable or writable)
     *  @return bool
     */
    bool active()
    {
        // wait for the socket
        return select(_socket + 1, &_set, &_set, nullptr, nullptr) > 0;
    }
};

/**
 *  End of namespace
 */
}

