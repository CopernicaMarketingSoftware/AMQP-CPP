/**
 *  PlatformName.h
 * 
 *  Class to extract the platform name (operating system, etc)
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2023 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <sys/utsname.h>

/**
 *  Begin of namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class PlatformName
{
private:
    /**
     *  The string holding all info
     *  @var std::string
     */
    std::string _value;

public:
    /**
     *  Constructor
     */
    PlatformName()
    {
        // all information
        struct utsname sysinfo;

        // retrieve system info
        if (uname(&sysinfo) != 0) return;
        
        // add all info
        _value.append(sysinfo.sysname).append(" ").append(sysinfo.version).append(" ").append(sysinfo.release).append(" running on ").append(sysinfo.nodename);
    }
    
    /**
     *  Destructor
     */
    virtual ~PlatformName() = default;
    
    /**
     *  Cast to a const char *
     *  @return const char *
     */
    operator const char * () const { return _value.data(); }
};

/**
 *  End of namespace
 */
}

