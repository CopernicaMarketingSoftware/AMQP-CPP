/**
 *  ProgramName.h
 * 
 *  Helper class that detects the name of the program
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
#include <limits.h>
#include <unistd.h>

/**
 *  Begin of namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class ProgramName
{
private:
    /**
     *  Path of the program
     *  @var char[]
     */
    char _path[PATH_MAX];
    
    /**
     *  Is the _path valid?
     *  @var bool
     */
    bool _valid;
    
public:
    /**
     *  Constructor
     */
    ProgramName()
    {
        // read the link target
        auto size = readlink("/proc/self/exe", _path, PATH_MAX);
        
        // -1 is returned on error, otherwise the size
        _valid = size >= 0;
        
        // set trailing null byte
        _path[size == PATH_MAX ? PATH_MAX-1 : size] = '\0';
    }
    
    /**
     *  Destructor
     */
    virtual ~ProgramName() = default;
    
    /**
     *  Cast to a const char *
     *  @return const char *
     */
    operator const char * () const
    {
        // empty string when not valid
        if (!_valid) return "";
        
        // return path to executable
        return _path;
    }
};

/**
 *  End of namespace
 */
}
