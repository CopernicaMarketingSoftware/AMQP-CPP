/**
 *  SslWrapper.h
 *
 *  Wrapper around a SSL pointer
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
class SslWrapper
{
private:
    /**
     *  The wrapped object
     *  @var SSL*
     */
    SSL *_ssl;
    
public:
    /**
     *  Constructor
     *  @param  ctx
     */
    SslWrapper(SSL_CTX *ctx) : _ssl(SSL_new(ctx)) 
    {
        // report error
        if (_ssl == nullptr) throw std::runtime_error("failed to construct ssl structure");
    }
    
    /**
     *  Wrapper constructor
     *  @param  ssl
     */
    SslWrapper(SSL *ssl) : _ssl(ssl)
    {
        // one more reference
        // @todo fix this
        //CRYPTO_add(_ssl);
    }
    
    /**
     *  Copy constructor
     *  @param  that
     */
    SslWrapper(const SslWrapper &that) : _ssl(that._ssl)
    {
        // one more reference
        // @todo fix this
        //SSL_up_ref(_ssl);
    }
    
    /**
     *  Destructor
     */
    virtual ~SslWrapper()
    {
        // destruct object
        SSL_free(_ssl);
    }
    
    /**
     *  Cast to the SSL*
     *  @return SSL *
     */
    operator SSL * () const { return _ssl; }
};

/**
 *  End of namespace
 */
}

