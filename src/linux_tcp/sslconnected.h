/**
 * SslConnected.h
 *
 * The actual tcp connection over SSL
 *
 * @copyright 2018 copernica BV
 */
 
/** 
  * Include guard
  */
#pragma once

/**
 * Dependencies
 */
#include "tcpoutbuffer.h"
#include "tcpinbuffer.h"
#include "wait.h"
#include "sslwrapper.h"
#include "sslshutdown.h"

/**
 * Set up namespace
 */
namespace AMQP {

/** 
 * Class definition
 */ 
class SslConnected : public TcpState, private Watchable 
{
private:
    /**
     *  The SSL structure
     *  @var SslWrapper
     */
    SslWrapper _ssl;

    /** 
     *  Socket file descriptor
     *  @var int
     */
    int _socket;
     
    /**
     *  The outgoing buffer
     *  @var TcpBuffer
     */
    TcpOutBuffer _out;

    /**
     *  The incoming buffer
     *  @var TcpInBuffer
     */
    TcpInBuffer _in;
    
    /**
     *  Are we now busy with sending or receiving?
     *  @var enum
     */
    enum State {
        state_idle,
        state_sending,
        state_receiving
    } _state;
    
    /**
     *  Should we close the connection after we've finished all operations?
     *  @var bool
     */
    bool _closed = false;
    
    /**
     *  Have we reported the final instruction to the user?
     *  @var bool
     */
    bool _finalized = false;

    /**
     *  Cached reallocation instruction
     *  @var size_t
     */
    size_t _reallocate = 0;
    

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
     *  Construct the final state
     *  @param  monitor     Object that monitors whether connection still exists
     *  @return TcpState*
     */
    TcpState *finalstate(const Monitor &monitor)
    {
        // close the socket if it is still open
        close();
        
        // if the object is still in a valid state, we can move to the close-state, 
        // otherwise there is no point in moving to a next state
        return monitor.valid() ? new TcpClosed(this) : nullptr;
    }
    
    /**
     *  Proceed with the next operation after the previous operation was
     *  a success, possibly changing the filedescriptor-monitor
     *  @return TcpState*
     */
    TcpState *proceed()
    {
        // if we still have an outgoing buffer we want to send out data
        if (_out)
        {
            // let's wait until the socket becomes writable
            _handler->monitor(_connection, _socket, readable | writable);
        }
        else if (_closed)
        {
            // start the state that closes the connection
            auto *nextstate = new SslShutdown(_connection, _socket, std::move(_ssl), _finalized, _handler);

            // we forget the current socket to prevent that it gets destructed
            _socket = -1;
            
            // report the next state
            return nextstate;
        }
        else
        {
            // let's wait until the socket becomes readable
            _handler->monitor(_connection, _socket, readable);
        }
        
        // done
        return this;
    }
    
    /**
     *  Method to repeat the previous call\
     *  @param  monitor     monitor to check if connection object still exists
     *  @param  state       the state that we were in
     *  @param  result      result of an earlier SSL_get_error call
     *  @return TcpState*
     */
    TcpState *repeat(const Monitor &monitor, enum State state, int error)
    {
        // check the error
        switch (error) {
        case SSL_ERROR_WANT_READ:
            // remember state
            _state = state;
            
            // the operation must be repeated when readable
            _handler->monitor(_connection, _socket, readable);
            
            // allow chaining
            return monitor.valid() ? this : nullptr;
        
        case SSL_ERROR_WANT_WRITE:
            // remember state
            _state = state;
            
            // wait until socket becomes writable again
            _handler->monitor(_connection, _socket, readable | writable);

            // allow chaining
            return monitor.valid() ? this : nullptr;

        case SSL_ERROR_NONE:
            // we're ready for the next instruction from userspace
            _state = state_idle;
            
            // turns out no error occured, an no action has to be rescheduled
            _handler->monitor(_connection, _socket, _out || _closed ? readable | writable : readable);

            // allow chaining
            return monitor.valid() ? this : nullptr;
            
        default:
            // if we have already reported an error to user space, we can go to the final state right away
            if (_finalized) return finalstate(monitor);
            
            // remember that we've sent out an error
            _finalized = true;
            
            // tell the handler
            _handler->onError(_connection, "ssl error");
            
            // go to the final state
            return finalstate(monitor);
        }
    }
    
    /**
     *  Parse the received buffer
     *  @param  monitor     object to check the existance of the connection object
     *  @param  size        number of bytes available
     *  @return TcpState
     */
    TcpState *parse(const Monitor &monitor, size_t size)
    {
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
        if (!_reallocate) return this;
        
        // reallocate the buffer
        _in.reallocate(_reallocate); 
        
        // we can remove the reallocate instruction
        _reallocate = 0;
        
        // done
        return this;
    }
    
    /**
     *  Perform a write operation
     *  @param  monitor         object to check the existance of the connection object
     *  @param  readable        is the connection also readable and should we call a read operation afterwards?
     *  @return TcpState*
     */
    TcpState *write(const Monitor &monitor, bool readable)
    {
        // assume default state
        _state = state_idle;
        
        // because the output buffer contains a lot of small buffers, we can do multiple 
        // operations till the buffer is empty (but only if the socket is not also
        // readable, because then we want to read that data first instead of endless writes
        do
        {
            // try to send more data from the outgoing buffer
            auto result = _out.sendto(_ssl);
            
            // we may have to repeat the operation on failure
            if (result > 0) continue;
            
            // the operation failed, we may have to repeat our call
            return repeat(monitor, state_sending, OpenSSL::SSL_get_error(_ssl, result));
        }
        while (_out && !readable);
        
        // proceed with the read operation or the event loop
        return readable ? receive(monitor, false) : proceed();
    }

    /**
     *  Perform a receive operation
     *  @param  monitor         object to check the existance of the connection object
     *  @param  writable        is the socket writable, and should we start a write operation after this operation?
     *  @return TcpState
     */
    TcpState *receive(const Monitor &monitor, bool writable)
    {
        // start a loop
        do
        {
            // assume default state
            _state = state_idle;

            // read data from ssl into the buffer
            auto result = _in.receivefrom(_ssl, _connection->expected());
            
            // if this is a failure, we are going to repeat the operation
            if (result <= 0) return repeat(monitor, state_receiving, OpenSSL::SSL_get_error(_ssl, result));
        
            // go process the received data
            auto *nextstate = parse(monitor, result);
            
            // leap out if we moved to a different state
            if (nextstate != this) return nextstate;
        }
        while (OpenSSL::SSL_pending(_ssl) > 0);
        
        // proceed with the write operation or the event loop
        return writable && _out ? write(monitor, false) : proceed();
    }

    
public:
    /**
     *  Constructor
     *  @param  connection  Parent TCP connection object
     *  @param  socket      The socket filedescriptor
     *  @param  ssl         The SSL structure
     *  @param  buffer      The buffer that was already built
     *  @param  handler     User-supplied handler object
     */
    SslConnected(TcpConnection *connection, int socket, SslWrapper &&ssl, TcpOutBuffer &&buffer, TcpHandler *handler) : 
        TcpState(connection, handler),
        _ssl(std::move(ssl)),
        _socket(socket),
        _out(std::move(buffer)),
        _in(4096),
        _state(_out ? state_sending : state_idle)
    {
        // tell the handler to monitor the socket if there is an out
        _handler->monitor(_connection, _socket, _state == state_sending ? readable | writable : readable); 
    }
    
    /**
     * Destructor
     */
    virtual ~SslConnected() noexcept
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
     *  @param  monitor     Object that can be used to find out if connection object is still alive
     *  @param  fd          The filedescriptor that is active
     *  @param  flags       AMQP::readable and/or AMQP::writable
     *  @return             New implementation object
     */
    virtual TcpState *process(const Monitor &monitor, int fd, int flags) override
    {
        // the socket must be the one this connection writes to
        if (fd != _socket) return this;
        
        // if we were busy with a write operation, we have to repeat that
        if (_state == state_sending) return write(monitor, flags & readable);
        
        // same is true for read operations, they should also be repeated
        if (_state == state_receiving) return receive(monitor, flags & writable);
        
        // if the socket is readable, we are going to receive data
        if (flags & readable) return receive(monitor, flags & writable);
        
        // socket is not readable (so it must be writable), do we have data to write?
        if (_out) return write(monitor, false);
        
        // the only scenario in which we can end up here is the socket should be
        // closed, but instead of moving to the shutdown-state right, we call proceed()
        // because that function is a little more careful
        return proceed();
    }

    /**
     *  Flush the connection, sent all buffered data to the socket
     *  @param  monitor     Object to check if connection still exists
     *  @return TcpState    new tcp state
     */
    virtual TcpState *flush(const Monitor &monitor) override
    {
        // we are not going to do this is object is busy reading
        if (_state == state_receiving) return this;
        
        // create an object to wait for the filedescriptor to becomes active
        Wait wait(_socket);
        
        // keep looping while we have an outgoing buffer
        while (_out)
        {
            // move to the idle-state
            _state = state_idle;
            
            // try to send more data from the outgoing buffer
            auto result = _out.sendto(_ssl);
            
            // was this a success?
            if (result > 0)
            {
                // proceed to the next state
                auto *nextstate = proceed();
                
                // leap out if we move to a different state
                if (nextstate != this) return nextstate;
            }
            else
            {
                // error was returned, so we must investigate what is going on
                auto error = OpenSSL::SSL_get_error(_ssl, result);
                
                // get the next state given the error
                auto *nextstate = repeat(monitor, state_sending, error);
                
                // leap out if we move to a different state
                if (nextstate != this) return nextstate;
                
                // check the type of error, and wait now
                switch (error) {
                case SSL_ERROR_WANT_READ:   wait.readable(); break;
                case SSL_ERROR_WANT_WRITE:  wait.active(); break;
                }
            }
        }
        
        // done
        return this;
    }

    /**
     *  Send data over the connection
     *  @param  buffer      buffer to send
     *  @param  size        size of the buffer
     */
    virtual void send(const char *buffer, size_t size) override
    {
        // put the data in the outgoing buffer
        _out.add(buffer, size);

        // if we're already busy with sending or receiving, we first have to wait
        // for that operation to complete before we can move on
        if (_state != state_idle) return;
        
        // let's wait until the socket becomes writable
        _handler->monitor(_connection, _socket, readable | writable);
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
     *  Report a connection error
     *  @param  error
     */
    virtual void reportError(const char *error) override
    {
        // we want to start the elegant ssl shutdown procedure, so we call reportClosed() here too,
        // because that function does exactly what we want to do here too
        reportClosed();
        
        // if the user was already notified of an final state, we do not have to proceed
        if (_finalized) return;
        
        // remember that this is the final call to user space
        _finalized = true;
        
        // pass to handler
        _handler->onError(_connection, error);
    }

    /**
     *  Report to the handler that the connection was nicely closed
     */
    virtual void reportClosed() override
    {
        // remember that the object is going to be closed
        _closed = true;
        
        // if the previous operation is still in progress we can wait for that
        if (_state != state_idle) return;
        
        // wait until the connection is writable so that we can close it then
        _handler->monitor(_connection, _socket, readable | writable);
    }
}; 

/**
 *  End of namespace
 */
}
