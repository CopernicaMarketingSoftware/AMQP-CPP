/**
 *  tcp.cpp
 *
 *  Implementation file for the tcp.h header
 */

/**
 *  Dependencies
 */
#include "includes.h"
#include "tcp.h"

/**
 *  Set up namespace
 */
namespace AMQP {

    namespace tcp {

        /*
         * https://msdn.microsoft.com/en-us/library/windows/desktop/ms737828(v=vs.85).aspx
         */
        int Errno() {
#if _WIN32 || _WIN64
            return WSAGetLastError();
#else
            return errno;
#endif
        }

        /*
        *  https://msdn.microsoft.com/en-us/library/windows/desktop/ms679351(v=vs.85).aspx
        */
        const char * const StrError(int errorCode) {
#if _WIN32 || _WIN64
            static thread_local std::string errorString;

            char *s = nullptr;
            FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, errorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPSTR)&s, 0, NULL);
            errorString = s;
            LocalFree(s);

            return errorString.c_str();
#else
            return strerror(errorCode);
#endif
        }

        /*
        * https://msdn.microsoft.com/en-us/library/windows/desktop/ms740126(v=vs.85).aspx
        */
        int Close(tcp::Socket socket) {
#if _WIN32 || _WIN64
            return closesocket(socket);
#else
            return close(socket);
#endif
        }


        /**
         *  End of namespace
         */
    }

}
