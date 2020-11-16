/**
 *  Address2Str.h
 * 
 *  Helper class to get a stringed version of an address obtained from get_addrinfo
 *  
 *  @author Aljar Meesters <aljar.meesters@copernica.com
 *  @copyright 2020 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <sys/socket.h>
#include <arpa/inet.h>

/**
 *  Opening namespace
 */
namespace AMQP {

/**
 *  class implemenation
 */
class Address2str
{
private:
    /**
     *  The address as string
     */
    char *_buffer = nullptr;

public:
    /**
     *  Constructor
     *  @param struct addrinfo *
     */
    Address2str(struct addrinfo *info)
    {
        // Switch on family
        switch(info->ai_family)
        {
            // If we are ip4
            case AF_INET: 
            {
                // ugly cast
                struct sockaddr_in *addr_in = (struct sockaddr_in *)(info->ai_addr);
                
                // create the buffer
                _buffer = new char[INET_ADDRSTRLEN];
                
                // get the info in our buffer
                inet_ntop(AF_INET, &(addr_in->sin_addr), _buffer, INET_ADDRSTRLEN);
                
                // done
                break;
            }

            // if we are ipv6
            case AF_INET6:
            {
                // ugly cast
                struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)(info->ai_addr);

                // crate buffer
                _buffer = new char[INET6_ADDRSTRLEN];

                // get the info in our buffer
                inet_ntop(AF_INET6, &(addr_in6->sin6_addr), _buffer, INET6_ADDRSTRLEN);
                
                // done
                break;
            }
            default:
                // If it is not ipv4 or ipv6 we don't set it.
                break;
        }
    }

    /**
     *  Destruction
     */
    virtual ~Address2str()
    {
        // if we have a buffer, we should free it.
        if (_buffer) delete[](_buffer);
    }

    /**
     *  get the char
     *  @return char*
     */
    const char *toChar() const
    {
        // expose the buffer.
        return _buffer;
    }
};


/**
 * ending of namespace
 */
}