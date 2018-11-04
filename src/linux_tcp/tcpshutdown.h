/**
 *  TcpShutdown.h
 *
 *  State in the TCP handshake that is responsible for gracefully
 *  shutting down the connection by closing our side of the connection,
 *  and waiting for the server to close the connection on the other
 *  side too.
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2018 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include "tcpextstate.h"

/**
 *  Begin of namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class TcpShutdown : public TcpExtState
{
protected:
    /**
     *  Method to report the result to the user
     */
    virtual void report()
    {
        // report that the connection was closed
        _parent->onClosed(this);
    }

public:
    /**
     *  Constructor
     *  @param  state       The previous state
     */
    TcpShutdown(TcpExtState *state) : TcpExtState(state) 
    {
        // we will shutdown the socket in a very elegant way, we notify the peer 
        // that we will not be sending out more write operations
        shutdown(_socket, SHUT_WR);
        
        // we still monitor the socket for readability to see if our close call was
        // confirmed by the peer
        _parent->onIdle(this, _socket, readable);
    }
    
    /**
     *  Forbidden to copy
     *  @param  that
     */
    TcpShutdown(const TcpShutdown &that) = delete;
    
    /**
     *  Destructor
     */
    virtual ~TcpShutdown() = default;
    
    /**
     *  Process the filedescriptor in the object
     *  @param  monitor     Monitor that can be used to check if the tcp connection is still alive
     *  @param  fd          The filedescriptor that is active
     *  @param  flags       AMQP::readable and/or AMQP::writable
     *  @return             New implementation object
     */
    virtual TcpState *process(const Monitor &monitor, int fd, int flags)
    {
        // must be the right filedescriptor
        if (_socket != fd) return this;
        
        // if the socket is not readable, we do not have to check anything
        if (!(flags & readable)) return this;
        
        // buffer to read data in
        char buffer[64];
        
        // read in data (we only do this to discover if the connection is really closed)
        auto result = read(_socket, buffer, sizeof(buffer));
        
        // if we read something, we keep on reading
        if (result > 0) return this;
        
        // or should we retry?
        if (result < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)) return this;

        // flush the connection to close the connection and report to the user
        return flush(monitor);
    }

    /**
     *  Flush the connection, make sure all network operations are finished
     *  @param  monitor     Object to check if connection still exists
     *  @return TcpState    New state
     */
    virtual TcpState *flush(const Monitor &monitor) override
    {
        // immediately close the socket
        cleanup();
        
        // report to the user that the operation is finished
        report();
        
        // move to next state
        return monitor.valid() ? new TcpClosed(this) : nullptr;
    }

    /**
     *  Abort the operation, immediately proceed to the final state
     *  @param  monitor     Monitor that can be used to check if the tcp connection is still alive
     *  @return TcpState    New implementation object
     */
    virtual TcpState *abort(const Monitor &monitor) override
    {
        // close the socket completely
        cleanup();
        
        // report the error to user-space
        // @todo do we have to report this?
        //_handler->onError(_connection, "tcp shutdown aborted");
        
        // move to next state
        return monitor.valid() ? new TcpClosed(this) : nullptr;
    }
};

/**
 *  End of namespace
 */
}

