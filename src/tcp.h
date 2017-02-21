/**
 *  tcp.h
 *
 *  Abstraction for windows and bsd sockets
 */

#pragma once

/**
 *  Dependencies
 */
#include "includes.h"
#include "../include/tcpdefines.h"

/**
 *  Set up namespace
 */
namespace AMQP {

namespace tcp
{

#if _WIN32 || _WIN64 
    constexpr tcp::Socket InvalidSocket = INVALID_SOCKET;
#else 
    constexpr tcp::Socket InvalidSocket = -1;
#endif

    /**
    *  Gets current networking error
    */
    int Errno();

    /**
    *  Formats network error code
    *  @param  errorCode  error code from errno
    */
    const char * const StrError(int errorCode);

    /**
    *  Closes socket
    *  @param  socket  to close
    */
    int Close(SOCKET socket);

};

/**
 *  End of namespace
 */
}

