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
 *  Function to convert error number to a string
 *  @param  retval  The retval from SSL_get_error
 *  @return  human-readable string
 */
static const char *errortostring(int retval)
{
    // determine the string to return
    switch (retval) {
    case SSL_ERROR_NONE:                 return "SSL_ERROR_NONE";
    case SSL_ERROR_ZERO_RETURN:          return "SSL_ERROR_ZERO_RETURN";
    case SSL_ERROR_WANT_READ:            return "SSL_ERROR_WANT_READ";
    case SSL_ERROR_WANT_WRITE:           return "SSL_ERROR_WANT_WRITE";
    case SSL_ERROR_WANT_CONNECT:         return "SSL_ERROR_WANT_CONNECT";
    case SSL_ERROR_WANT_ACCEPT:          return "SSL_ERROR_WANT_ACCEPT";
    case SSL_ERROR_WANT_X509_LOOKUP:     return "SSL_ERROR_WANT_X509_LOOKUP";
    case SSL_ERROR_WANT_ASYNC:           return "SSL_ERROR_WANT_ASYNC";
    case SSL_ERROR_WANT_ASYNC_JOB:       return "SSL_ERROR_WANT_ASYNC_JOB";
    case SSL_ERROR_WANT_CLIENT_HELLO_CB: return "SSL_ERROR_WANT_CLIENT_HELLO_CB";
    default:                             return "unknown ssl error";
    }
}

/**
 *  Begin namespace
 */
namespace AMQP {

/**
 *  Callback used for ERR_print_errors_cb
 *  @param  str   The string
 *  @param  len   The length
 *  @param  ctx   The context (this ptr)
 *  @return       always 0 to signal to OpenSSL to continue
 */
int sslerrorprintercallback(const char *str, size_t len, void *ctx)
{
    // Cast to ourselves and store the error line. OpenSSL adds a newline to every error line.
    static_cast<SslErrorPrinter*>(ctx)->_message.append(str, len);

    // continue with the next message
    return 0;
}

/**
 *  Constructor
 *  @param retval return value of SSL_get_error (must be a proper error)
 */
SslErrorPrinter::SslErrorPrinter(int retval)
{
    // check the return value of the SSL_get_error function, which has a very unfortunate name.
    switch (retval)
    {
    // It can be a syscall error.
    case SSL_ERROR_SYSCALL:

        // The SSL_ERROR_SYSCALL with errno value of 0 indicates unexpected
        // EOF from the peer. This will be properly reported as SSL_ERROR_SSL
        // with reason code SSL_R_UNEXPECTED_EOF_WHILE_READING in the
        // OpenSSL 3.0 release because it is truly a TLS protocol error to
        // terminate the connection without a SSL_shutdown().
        if (errno == 0) _message = "SSL_R_UNEXPECTED_EOF_WHILE_READING";

        // Otherwise we ask the OS for a description of the error.
        else _message = ::strerror(errno);

        // done
        break;

    // It can be an error in OpenSSL. In that case the error stack contains
    // more information. The documentation notes: if this error occurs then
    // no further I/O operations should be performed on the connection and
    // SSL_shutdown() must not be called.
    case SSL_ERROR_SSL:

        // collect all error lines
        OpenSSL::ERR_print_errors_cb(&sslerrorprintercallback, this);

        // remove the last newline
        if (!_message.empty() && _message.back() == '\n') _message.pop_back();

        // done
        break;

    default:
        // we don't know what kind of error this is
        _message = errortostring(retval);

        // done
        break;
    }
}

/**
 *  data ptr (guaranteed null-terminated)
 *  @return const char *
 */
const char *SslErrorPrinter::data() const noexcept { return _message.data(); }

/**
 *  length of the string
 *  @return size_t
 */
std::size_t SslErrorPrinter::size() const noexcept { return _message.size(); }

/**
 *  End of namespace AMQP
 */
}
