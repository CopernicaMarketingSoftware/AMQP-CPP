/**
 *  SslErrorPrinter.h
 *
 *  Flushes the SSL error stack to a BIO.
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
    /**
     *  pointer to the BIO
     *  @var unique_ptr
     */
    std::unique_ptr<::BIO, decltype(&::BIO_free)> _bio;

    /**
     *  pointer to the (data, size) tuple
     *  @var ::BUF_MEM*
     */
    ::BUF_MEM *_bufmem = nullptr;

    /**
     *  In case of a syscall error, the static string pointing to the error
     *  @var const char*
     */
    const char *_strerror = nullptr;

public:
    /**
     *  Constructor
     *  @param retval return value of SSL_get_error (must be a proper error)
     *  @throw std::bad_alloc if the BIO couldn't be allocated
     */
    SslErrorPrinter(int retval);

    /**
     *  data ptr
     *  @return const char *
     */
    const char *data() const noexcept;

    /**
     *  length of the string
     *  @return size_t
     */
    std::size_t size() const noexcept;
};

/**
 *  End of namespace AMQP
 */
}
