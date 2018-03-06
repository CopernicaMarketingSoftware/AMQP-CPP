/**
 *  SslContext.h
 *
 *  Class to create and maintain a tcp ssl context
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
class SslContext
{
private:
    /**
     *  The wrapped context
     *  @var SSL_CTX
     */
    SSL_CTX *_ctx;

public:
    /**
     *  Constructor
     *  @param  method
     *  @throws std::runtime_error
     */
    SslContext(const SSL_METHOD *method) : _ctx(SSL_CTX_new(method)) 
    {
        // report error
        if (_ctx == nullptr) throw std::runtime_error("failed to construct ssl context");
    }
    
    /**
     *  Constructor that wraps around an existing context
     *  @param  context
     */
    SslContext(SSL_CTX *context) : _ctx(context)
    {
        // increment refcount
        // @todo fix this
        //SSL_ctx_up_ref(context);
    }
    
    /**
     *  Copy constructor
     *  @param  that
     */
    SslContext(SslContext &that) : _ctx(that._ctx)
    {
        // increment refcount
        // @todo fix this
        //SSL_ctx_up_ref(context);
    }
    
    /**
     *  Destructor
     */
    virtual ~SslContext()
    {
        // free resource (this updates the refcount -1, and may destruct it)
        SSL_CTX_free(_ctx);
    }
    
    /**
     *  Cast to the actual context
     *  @return SSL_CTX *
     */
    operator SSL_CTX * () { return _ctx; }
};

/**
 *  End of namespace
 */
}

