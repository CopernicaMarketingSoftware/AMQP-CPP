/**
 *  OpenSSL.cpp
 *
 *  Implementation file for the openssl.h header file
 *
 *  @copyright 2018 Copernica BV
 */

/**
 *  Dependencies
 */
#include "openssl.h"
#include "function.h"
#include "library.h"

/**
 *  Begin of namespace
 */
namespace AMQP { namespace OpenSSL {

/**
 *  Get the library handle
 *  @return void *
 */
static void *library()
{
    // create on instance
    static Library instance;
    
    // return the instance (it has a cast-to-void-ptr operator)
    return instance;
}

/**
 *  Create new SSL context
 *  @param  method  SSL_METHOD can be of the following types: TLS_method(), TLS_server_method(), TLS_client_method()
 *  @return         pointer to object   
 */
SSL_CTX *SSL_CTX_new(const SSL_METHOD *method) 
{
    // create a function
    static Function<decltype(::SSL_CTX_new)> func(library(), "SSL_CTX_new");
    
    // call the openssl function
    return func(method);
}

/**
 *  Read data from an ssl socket
 *  @param  ssl     ssl structure
 *  @param  buf     buffer to read into
 *  @param  num     size of buffer
 *  @return int     number of bytes read
 */
int SSL_read(SSL *ssl, void *buf, int num)
{
    // create a function
    static Function<decltype(::SSL_read)> func(library(), "SSL_read");
    
    // call the openssl function
    return func(ssl, buf, num);
}

/**
 *  Read data from an ssl socket
 *  @param  ssl     ssl structure
 *  @param  buf     buffer to write
 *  @param  num     size of buffer
 *  @return int     number of bytes written
 */
int SSL_write(SSL *ssl, const void *buf, int num)
{
    // create a function
    static Function<decltype(::SSL_write)> func(library(), "SSL_write");
    
    // call the openssl function
    return func(ssl, buf, num);
}

/**
 *  Connect the SSL object with a file descriptor
 *  @param  ssl     SSL object
 *  @param  fd      file descriptor
 *  @return int     wether the operation succeeded or not   
 */
int SSL_set_fd(SSL *ssl, int fd) 
{
    // create a function
    static Function<decltype(::SSL_set_fd)> func(library(), "SSL_set_fd");
    
    // call the openssl function
    return func(ssl, fd);
}


/**
 *  Free an allocated SSL structure
 *  @param  ssl     SSL object to be freed
 *  @return int     wether the operation succeeded or not   
 */
void SSL_free(SSL *ssl)
{
    // create a function
    static Function<decltype(::SSL_free)> func(library(), "SSL_free");
    
    // call the openssl function
    return func(ssl);
}

/**
 *  Create a new SSL structure for a connection
 *  @param  ctx     SSL context object
 *  @return SSL     the created SSL oject based on th context   
 */
SSL *SSL_new(SSL_CTX *ctx)
{
    // create a function
    static Function<decltype(::SSL_new)> func(library(), "SSL_new");
    
    // call the openssl function
    return func(ctx);
}

/**
 *  Increment refcount for a ssl structure
 *  @param  ctx     SSL structure
 *  @return int     1 for success, 0 for failure   
 */
int SSL_up_ref(SSL *ssl)
{
    // create a function
    static Function<decltype(SSL_up_ref)> func(library(), "SSL_up_ref");
    
    // call the openssl function if it exists
    if (func) return func(ssl);
    
    // @todo use our own implementation
    
    return 0;
}

/**
 *  Shut down a TLS/SSL shut down
 *  @param  ssl     SSL object to terminate
 *  @return int     returns diagnostic values   
 */
int SSL_shutdown(SSL *ssl)
{
    // create a function
    static Function<decltype(::SSL_shutdown)> func(library(), "SSL_shutdown");
    
    // call the openssl function
    return func(ssl);
}

/**
 *  Prepare SSL object to work in client or server mode
 *  @param  ssl     SSL object to set connect state on              
 */
void SSL_set_connect_state(SSL *ssl)
{
    // create a function
    static Function<decltype(::SSL_set_connect_state)> func(library(), "SSL_set_connect_state");
    
    // call the openssl function
    func(ssl);

}

/**
 *  Perform a TLS/SSL handshake
 *  @param  ssl     SSL object
 *  @return int     returns diagnostic values   
 */
int SSL_do_handshake(SSL *ssl)
{
    // create a function
    static Function<decltype(::SSL_do_handshake)> func(library(), "SSL_do_handshake");
    
    // call the openssl function
    return func(ssl);
}

/**
 *  Obtain result code for TLS/SSL I/O operation
 *  @param ssl      SSL object
 *  @param ret      the returned diagnostic value of SSL calls
 *  @return int     returns error values
 */
int SSL_get_error(const SSL *ssl, int ret)
{
    // create a function
    static Function<decltype(::SSL_get_error)> func(library(), "SSL_get_error");
    
    // call the openssl function
    return func(ssl, ret);
}
    
/**
 *  End of namespace
 */
}}

