/**
 *  SslErrorPrinter.h
 *
 *  Flushes the SSL error stack to a string.
 *  You can get at the string content via the data() and size() methods.
 *  After constructing an instance of this class, the SSL error stack
 *  is empty.
 *
 *  @copyright 2021 copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include "openssl.h"
#include <string>
#include <memory>

/**
 *  Begin namespace
 */
namespace AMQP {

/**
 *  Class declaration
 */
class SslErrorPrinter final
{
public:
    /**
     *  Constructor
     *  @param retval return value of SSL_get_error (must be a proper error)
     */
    SslErrorPrinter(int retval);

    /**
     *  data ptr (guaranteed null-terminated)
     *  @return const char *
     */
    const char *data() const noexcept;

    /**
     *  length of the string
     *  @return size_t
     */
    std::size_t size() const noexcept;

private:
    /**
     *  The error message
     *  @var std::string
     */
    std::string _message;

    /**
     *  Callback used for ERR_print_errors_cb
     *  @param  str   The string
     *  @param  len   The length
     *  @param  ctx   The context (this ptr)
     *  @return       always 0 to signal to OpenSSL to continue
     */
    friend int sslerrorprintercallback(const char *str, size_t len, void *ctx);
};

/**
 *  End of namespace AMQP
 */
}
