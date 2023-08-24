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
#if defined(_WIN32) || defined(_WIN64)
#include "Windows.h"
#define PATH_MAX MAX_PATH
#else
#include <unistd.h>
#endif

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
#if defined(_WIN32) || defined(_WIN64)
        // the the 
        auto size = GetModuleFileNameA(NULL, _path, PATH_MAX);
        
        // -1 is returned on error, otherwise the size
        _valid = size >= 0;
        
        // set trailing null byte
        _path[size == PATH_MAX ? PATH_MAX-1 : size] = '\0';
#else
        // read the link target
        auto size = readlink("/proc/self/exe", _path, PATH_MAX);
        
        // -1 is returned on error, otherwise the size
        _valid = size >= 0;
        
        // set trailing null byte
        _path[size == PATH_MAX ? PATH_MAX-1 : size] = '\0';
#endif
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
