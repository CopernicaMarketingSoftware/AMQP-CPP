/**
 *  TcpSsl.h
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
class TcpSsl
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
    TcpSsl(SSL_CTX *ctx) : _ssl(SSL_new(ctx)) 
    {
        // report error
        if (_ssl == nullptr) throw std::runtime_error("failed to construct ssl structure");
    }
    
    /**
     *  Wrapper constructor
     *  @param  ssl
     */
    TcpSsl(SSL *ssl) : _ssl(ssl)
    {
        // one more reference
        // @todo fix this
        //CRYPTO_add(_ssl);
    }
    
    /**
     *  Copy constructor
     *  @param  that
     */
    TcpSsl(const TcpSsl &that) : _ssl(that._ssl)
    {
        // one more reference
        // @todo fix this
        //SSL_up_ref(_ssl);
    }
    
    /**
     *  Destructor
     */
    virtual ~TcpSsl()
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

