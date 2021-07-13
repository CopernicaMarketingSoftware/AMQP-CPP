/**
 *  SslErrorPrinter.cpp
 *
 *  Implementation of SslErrorPrinter
 *
 *  @copyright 2021 copernica BV
 */

/**
 *  Dependencies
 */
#include "sslerrorprinter.h"
#include <cstring>

/**
 *  Begin namespace
 */
namespace AMQP {

/**
 *  Constructor
 *  @param retval return value of SSL_get_error (must be a proper error)
 *  @throw std::bad_alloc if the BIO couldn't be allocated
 */
SslErrorPrinter::SslErrorPrinter(int retval) : _bio(nullptr, &::BIO_free)
{
    // check the return value of the SSL_get_error function, which has a very unfortunate name.
    switch (retval)
    {
        // It can be a syscall error.
        case SSL_ERROR_SYSCALL:
        {
            // The SSL_ERROR_SYSCALL with errno value of 0 indicates unexpected
            // EOF from the peer. This will be properly reported as SSL_ERROR_SSL
            // with reason code SSL_R_UNEXPECTED_EOF_WHILE_READING in the
            // OpenSSL 3.0 release because it is truly a TLS protocol error to
            // terminate the connection without a SSL_shutdown().
            if (errno == 0) _strerror = "SSL_R_UNEXPECTED_EOF_WHILE_READING";

            // Otherwise we ask the OS for a description of the error.
            else _strerror = ::strerror(errno);

            // done
            break;
        }
        // It can be an error in OpenSSL. In that case the error stack contains
        // more information. The documentation notes: if this error occurs then
        // no further I/O operations should be performed on the connection and
        // SSL_shutdown() must not be called.
        case SSL_ERROR_SSL:
        {
            // create a new bio
            _bio = decltype(_bio)(::BIO_new(::BIO_s_mem()), &::BIO_free);

            // check if it was allocated
            if (!_bio) throw std::bad_alloc();

            // invoke the convenience function to extract the whole error stack
            ::ERR_print_errors(_bio.get());

            // get it from the bio
            ::BIO_get_mem_ptr(_bio.get(), &_bufmem);

            // done
            break;
        }
        default:
        {
            // we don't know what kind of error this is
            _strerror = "unknown ssl error";

            // done
            break;
        }
    }
}

/**
 *  data ptr
 *  @return const char *
 */
const char *SslErrorPrinter::data() const noexcept { return _strerror ? _strerror : _bufmem->data; }

/**
 *  length of the string
 *  @return size_t
 */
std::size_t SslErrorPrinter::size() const noexcept { return _strerror ? std::strlen(_strerror) : _bufmem->length; }

/**
 *  End of namespace AMQP
 */
}
