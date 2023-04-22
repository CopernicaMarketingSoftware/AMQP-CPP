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
#include <unistd.h>
#include <limits.h>

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
    ProgramName() : _valid(readlink("/proc/self/exe", _path, PATH_MAX) == 0) {}
    
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
        
        // locate the last slash
        auto *slash = strrchr(_path, '/');
        
        // if not found return entire path, otherwise just the program name
        return slash ? slash + 1 : _path;
    }
};

/**
 *  End of namespace
 */
}
