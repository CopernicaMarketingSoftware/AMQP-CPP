/**
 *  TcpConnected.h
 * 
 *  The actual tcp connection - this is the "_impl" of a tcp-connection after
 *  the hostname was resolved into an IP address
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include "tcpbuffer.h"

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class TcpConnected : public TcpState, private Watchable
{
private:
    /**
     *  The socket file descriptor
     *  @var int
     */
    int _socket;
    
    /**
     *  The outgoing buffer
     *  @var TcpBuffer
     */
    TcpBuffer _out;
    
    /**
     *  An incoming buffer
     *  @var TcpBuffer
     */
    TcpBuffer _in;
    
    
    /**
     *  Helper method to report an error
     *  @return bool        Was an error reported?
     */
    bool reportError()
    {
        // some errors are ok and do not (necessarily) mean that we're disconnected
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) return false;
        
        // we have an error - report this to the user
        _handler->onError(_connection, strerror(errno));
        
        // done
        return true;
    }
    
    /**
     *  Construct the next state
     *  @param  monitor     Object that monitors whether connection still exists
     *  @return TcpState*
     */
    TcpState *nextState(const Monitor &monitor)
    {
        // if the object is still in a valid state, we can move to the close-state, 
        // otherwise there is no point in moving to a next state
        return monitor.valid() ? new TcpClosed(this) : nullptr;
    }
    
    
public:
    /**
     *  Constructor
     *  @param  connection  Parent TCP connection object
     *  @param  socket      The socket filedescriptor
     *  @param  buffer      The buffer that was already built
     *  @param  handler     User-supplied handler object
     */
    TcpConnected(TcpConnection *connection, int socket, TcpBuffer &&buffer, TcpHandler *handler) : 
        TcpState(connection, handler),
        _socket(socket),
        _out(std::move(buffer))
    {
        // if there is already an output buffer, we have to send out that first
        if (_out) _out.sendto(_socket);
        
        // tell the handler to monitor the socket, if there is an out
        _handler->monitor(_connection, _socket, _out ? readable | writable : readable);
    }
    
    /**
     *  Destructor
     */
    virtual ~TcpConnected()
    {
        // we no longer have to monitor the socket
        _handler->monitor(_connection, _socket, 0);
        
        // close the socket
        close(_socket);
    }
    
    /**
     *  Process the filedescriptor in the object
     *  @param  fd          Filedescriptor that is active
     *  @param  flags       AMQP::readable and/or AMQP::writable
     *  @return             New state object
     */
    virtual TcpState *process(int fd, int flags) override
    {
        // must be the socket
        if (fd != _socket) return this;

        // because the object might soon be destructed, we create a monitor to check this
        Monitor monitor(this);
        
        // can we write more data to the socket?
        if (flags & writable)
        {
            // send out the buffered data
            auto result = _out.sendto(_socket);
            
            // are we in an error state?
            if (result <  0 && reportError()) return nextState(monitor);
            
            // if buffer is empty by now, we no longer have to check for 
            // writability, but only for readability
            if (!_out) _handler->monitor(_connection, _socket, readable);
        }
        
        // should we check for readability too?
        if (flags & readable)
        {
            // read data from buffer
            auto result = _in.receivefrom(_socket);
            
            // are we in an error state?
            if (result < 0 && reportError()) return nextState(monitor);
            
            // we need a local copy of the buffer - because it is possible that "this"
            // object gets destructed halfway through the call to the parse() method
            TcpBuffer buffer(std::move(_in));
            
            // parse the buffer
            auto processed = _connection->parse(buffer);

            // "this" could be removed by now, check this
            if (!monitor.valid()) return nullptr;
            
            // shrink buffer
            buffer.shrink(processed);
            
            // restore the buffer as member
            std::swap(_in, buffer);
        }
        
        // keep same object
        return this;
    }

    /**
     *  Send data over the connection
     *  @param  buffer      buffer to send
     *  @param  size        size of the buffer
     */
    virtual void send(const char *buffer, size_t size) override
    {
        // is there already a buffer of data that can not be sent?
        if (_out) return _out.add(buffer, size);
        
        // there is no buffer, send the data right away
        auto result = write(_socket, buffer, size);
    
        // number of bytes sent
        size_t bytes = result < 0 ? 0 : result;

        // ok if all data was sent
        if (bytes >= size) return;
    
        // add the data to the buffer
        _out.add(buffer + bytes, size - bytes);
        
        // start monitoring the socket to find out when it is writable
        _handler->monitor(_connection, _socket, readable | writable);
    }
};
    
/**
 *  End of namespace
 */
}

