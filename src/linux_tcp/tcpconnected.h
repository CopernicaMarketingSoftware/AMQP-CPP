/**
 *  TcpConnected.h
 * 
 *  The actual tcp connection - this is the "_impl" of a tcp-connection after
 *  the hostname was resolved into an IP address
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 - 2018 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include "tcpoutbuffer.h"
#include "tcpinbuffer.h"
#include "wait.h"

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
     *  @var TcpOutBuffer
     */
    TcpOutBuffer _out;
    
    /**
     *  An incoming buffer
     *  @var TcpInBuffer
     */
    TcpInBuffer _in;
    
    /**
     *  Cached reallocation instruction
     *  @var size_t
     */
    size_t _reallocate = 0;
    
    /**
     *  Have we already made the last report to the user (about an error or closed connection?)
     *  @var bool
     */
    bool _finalized = false;

    
    /**
     *  Close the connection
     *  @return bool
     */
    bool close()
    {
        // do nothing if already closed
        if (_socket < 0) return false;
        
        // and stop monitoring it
        _handler->monitor(_connection, _socket, 0);

        // close the socket
        ::close(_socket);
        
        // forget filedescriptor
        _socket = -1;
        
        // done
        return true;
    }
    
    /**
     *  Helper method to report an error
     *  @return bool        Was an error reported?
     */
    bool reportError()
    {
        // some errors are ok and do not (necessarily) mean that we're disconnected
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) return false;
        
        // connection can be closed now
        close();
        
        // if the user has already been notified, we do not have to do anything else
        if (_finalized) return true;
        
        // update the _finalized member before we make the call to user space because
        // the user space may destruct this object
        _finalized = true;
        
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
    TcpConnected(TcpConnection *connection, int socket, TcpOutBuffer &&buffer, TcpHandler *handler) : 
        TcpState(connection, handler),
        _socket(socket),
        _out(std::move(buffer)),
        _in(4096)
    {
        // if there is already an output buffer, we have to send out that first
        if (_out) _out.sendto(_socket);
        
        // tell the handler to monitor the socket, if there is an out
        _handler->monitor(_connection, _socket, _out ? readable | writable : readable);
    }
    
    /**
     *  Destructor
     */
    virtual ~TcpConnected() noexcept
    {
        // close the socket
        close();
    }

    /**
     *  The filedescriptor of this connection
     *  @return int
     */
    virtual int fileno() const override { return _socket; }

    /**
     *  Number of bytes in the outgoing buffer
     *  @return std::size_t
     */
    virtual std::size_t queued() const override { return _out.size(); }

    /**
     *  Process the filedescriptor in the object
     *  @param  monitor     Monitor to check if the object is still alive
     *  @param  fd          Filedescriptor that is active
     *  @param  flags       AMQP::readable and/or AMQP::writable
     *  @return             New state object
     */
    virtual TcpState *process(const Monitor &monitor, int fd, int flags) override
    {
        // must be the socket
        if (fd != _socket) return this;

        // can we write more data to the socket?
        if (flags & writable)
        {
            // send out the buffered data
            auto result = _out.sendto(_socket);
            
            // are we in an error state?
            if (result < 0 && reportError()) return nextState(monitor);
            
            // if buffer is empty by now, we no longer have to check for 
            // writability, but only for readability
            if (!_out) _handler->monitor(_connection, _socket, readable);
        }
        
        // should we check for readability too?
        if (flags & readable)
        {
            // read data from buffer
            ssize_t result = _in.receivefrom(_socket, _connection->expected());
            
            // are we in an error state?
            if (result < 0 && reportError()) return nextState(monitor);

            // we need a local copy of the buffer - because it is possible that "this"
            // object gets destructed halfway through the call to the parse() method
            TcpInBuffer buffer(std::move(_in));
            
            // parse the buffer
            auto processed = _connection->parse(buffer);

            // "this" could be removed by now, check this
            if (!monitor.valid()) return nullptr;
            
            // shrink buffer
            buffer.shrink(processed);
            
            // restore the buffer as member
            _in = std::move(buffer);
            
            // do we have to reallocate?
            if (_reallocate) _in.reallocate(_reallocate); 
            
            // we can remove the reallocate instruction
            _reallocate = 0;
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
        auto result = ::send(_socket, buffer, size, AMQP_CPP_MSG_NOSIGNAL);

        // number of bytes sent
        size_t bytes = result < 0 ? 0 : result;

        // ok if all data was sent
        if (bytes >= size) return;
    
        // add the data to the buffer
        _out.add(buffer + bytes, size - bytes);
        
        // start monitoring the socket to find out when it is writable
        _handler->monitor(_connection, _socket, readable | writable);
    }
    
    /**
     *  Flush the connection, sent all buffered data to the socket
     *  @param  monitor     Object to check if connection still lives
     *  @return TcpState    new tcp state
     */
    virtual TcpState *flush(const Monitor &monitor) override
    {
        // create an object to wait for the filedescriptor to becomes active
        Wait wait(_socket);

        // keep running until the out buffer is not empty
        while (_out)
        {
            // poll the socket, is it already writable?
            if (!wait.writable()) return this;
            
            // socket is writable, send as much data as possible
            auto *newstate = process(monitor, _socket, writable);
            
            // are we done
            if (newstate != this) return newstate;
        }
        
        // all has been sent
        return this;
    }

    /**
     *  Report that heartbeat negotiation is going on
     *  @param  heartbeat   suggested heartbeat
     *  @return uint16_t    accepted heartbeat
     */
    virtual uint16_t reportNegotiate(uint16_t heartbeat) override
    {
        // remember that we have to reallocate (_in member can not be accessed because it is moved away)
        _reallocate = _connection->maxFrame();
        
        // pass to base
        return TcpState::reportNegotiate(heartbeat);
    }

    /**
     *  Report to the handler that the object is in an error state.
     *  @param  error
     */
    virtual void reportError(const char *error) override
    {
        // close the socket
        close();
        
        // if the user was already notified of an final state, we do not have to proceed
        if (_finalized) return;
        
        // remember that this is the final call to user space
        _finalized = true;
        
        // pass to handler
        _handler->onError(_connection, error);
    }

    /**
     *  Report to the handler that the connection was nicely closed
     *  This is the counter-part of the connection->close() call.
     */
    virtual void reportClosed() override
    {
        // we will shutdown the socket in a very elegant way, we notify the peer 
        // that we will not be sending out more write operations
        shutdown(_socket, SHUT_WR);
        
        // we still monitor the socket for readability to see if our close call was
        // confirmed by the peer
        _handler->monitor(_connection, _socket, readable);

        // if the user was already notified of an final state, we do not have to proceed
        if (_finalized) return;
        
        // remember that this is the final call to user space
        _finalized = true;
        
        // pass to handler
        _handler->onClosed(_connection);
    }
};
    
/**
 *  End of namespace
 */
}

