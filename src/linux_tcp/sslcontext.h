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
     *  @param  method      the connect method to use
     *  @throws std::runtime_error
     */
    SslContext(const SSL_METHOD *method) : _ctx(OpenSSL::SSL_CTX_new(method)) 
    {
        // report error
        if (_ctx == nullptr) throw std::runtime_error("failed to construct ssl context");
    }
    
    /**
     *  Copy constructor is delete because the object is refcounted,
     *  and we do not have a decent way to update the refcount in openssl 1.0
     *  @param  that
     */
    SslContext(SslContext &that) = delete;
    
    /**
     *  Destructor
     */
    virtual ~SslContext()
    {
        // free resource (this updates the refcount -1, and may destruct it)
        OpenSSL::SSL_CTX_free(_ctx);
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

