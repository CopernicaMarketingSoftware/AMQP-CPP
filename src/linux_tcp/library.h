/**
 *  Library.h
 * 
 *  The Library class is a wrapper around dlopen()
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
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
class Library
{
private:
    /**
     *  The library handle
     *  @var void *
     */
    void *_handle;
    
public:
    /**
     *  Constructor
     */
    Library() : _handle(dlopen(nullptr, RTLD_NOW)) {}
    
    /**
     *  No copying
     *  @param  that
     */
    Library(const Library &that) = delete;
    
    /**
     *  Destructor
     */
    virtual ~Library()
    {
        // close library
        if (_handle) dlclose(_handle);
    }
    
    /**
     *  Cast to the handle
     *  @return void *
     */
    operator void * () { return _handle; }
};
    
/**
 *  End of namespace
 */
}

