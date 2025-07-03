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
    SSL_CTX *_ctx = nullptr;

    /**
     *  If set to true, the SSL context is owned by this wrapper and will be
     *  freed by the destructor.
     *  @var owned
     */
    bool _owned = false;

public:
    /**
     *  Constructor accepting a preconfigured SSL_CTX.
     *  @param  ctx         preconfigured SSL_CTX to use
     *  @throws std::runtime_error  if ctx is null
     */
    explicit SslContext(SSL_CTX *ctx) : _ctx(ctx)
    {
        // report error
        if (_ctx == nullptr) throw std::runtime_error("no ssl context");
    }

    /**
     *  Constructor creating a new SSL_CTX wuth default verification paths.
     *  @param  method      the connect method to use
     *  @throws std::runtime_error
     */
    explicit SslContext(const SSL_METHOD *method) : _ctx(OpenSSL::SSL_CTX_new(method)), _owned(true)
    {
        // report error
        if (_ctx == nullptr) throw std::runtime_error("failed to construct ssl context");

        // use the default directories for verifying certificates
        OpenSSL::SSL_CTX_set_default_verify_paths(_ctx);
    }
    
    /**
     *  Copy constructor is delete because the object is refcounted,
     *  and we do not have a decent way to update the refcount in openssl 1.0
     *  @param  that
     */
    SslContext(const SslContext &that) = delete;
    SslContext(SslContext &&that) noexcept = default;
    
    /**
     *  Destructor
     */
    virtual ~SslContext()
    {
        // free resource (this updates the refcount -1, and may destruct it)
        if (_owned) OpenSSL::SSL_CTX_free(_ctx);
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

