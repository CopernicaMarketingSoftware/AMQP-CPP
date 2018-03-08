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
    enum {
        state_idle,
        state_sending,
        state_receiving
    } _state;
    
    /**
     *  Is the object already closed?
     *  @var bool
     */
    bool _closed = false;

    /**
     *  Cached reallocation instruction
     *  @var size_t
     */
    size_t _reallocate = 0;
    

    /**
     *  Helper method to report an error
     *  @return bool        Was an error reported?
     */
    bool reportError()
    {
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
            // we still have a buffer with outgoing data
            _state = state_sending;
        
            // let's wait until the socket becomes writable
            _handler->monitor(_connection, _socket, readable | writable);
        }
        else if (_closed)
        {
            // we forget the current handler to prevent that things are changed
            _handler = nullptr;
            
            // start the state that closes the connection
            return new SslShutdown(_connection, _socket, std::move(_ssl), _handler);
        }
        else
        {
            // outgoing buffer is empty, we're idle again waiting for further input
            _state = state_idle;
            
            // let's wait until the socket becomes readable
            _handler->monitor(_connection, _socket, readable);
        }
        
        // done
        return this;
    }
    
    /**
     *  Method to repeat the previous call
     *  @param  result      result of an earlier openssl operation
     *  @return TcpState*
     */
    TcpState *repeat(int result)
    {
        // error was returned, so we must investigate what is going on
        auto error = OpenSSL::SSL_get_error(_ssl, result);
        
        // create a monitor because the handler could make things ugly
        Monitor monitor(this);
        
        // check the error
        switch (error) {
        case SSL_ERROR_WANT_READ:
            // the operation must be repeated when readable
            _handler->monitor(_connection, _socket, readable);
            
            // allow chaining
            return monitor.valid() ? this : nullptr;
        
        case SSL_ERROR_WANT_WRITE:
            // wait until socket becomes writable again
            _handler->monitor(_connection, _socket, readable | writable);

            // allow chaining
            return monitor.valid() ? this : nullptr;

        case SSL_ERROR_NONE:
            // turns out no error occured, an no action has to be rescheduled
            _handler->monitor(_connection, _socket, _out ? readable | writable : readable);

            // we're ready for the next instruction from userspace
            _state = state_idle;

            // allow chaining
            return monitor.valid() ? this : nullptr;
            
        default:
            // is the peer trying to shutdown? (we dont expect this)
            bool shutdown = OpenSSL::SSL_get_shutdown(_ssl);

            // send back a nice shutdown
            if (shutdown) OpenSSL::SSL_shutdown(_ssl);
                
            // tell the handler
            _handler->onError(_connection, "ssl error");
            
            // no need to chain if object is already destructed
            if (!monitor) return nullptr;
            
            // create a new new object
            //return shutdown ? 
            
            // allow chaining
            return nullptr; //monitor.valid() ? new TcpClosed(this) : nullptr;
        }
    }
    
    /**
     *  Parse the received buffer
     *  @param  size
     *  @return TcpState
     */
    TcpState *parse(size_t size)
    {
        // we need a local copy of the buffer - because it is possible that "this"
        // object gets destructed halfway through the call to the parse() method
        TcpInBuffer buffer(std::move(_in));
        
        // because the object might soon be destructed, we create a monitor to check this
        Monitor monitor(this);
        
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
        
        // done
        return this;
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
        _handler->monitor(_connection, _socket, _state == state_sending ? writable : readable); 
    }   
    
    /**
     * Destructor
     */
    virtual ~SslConnected() noexcept
    {
        // skip if handler is already forgotten
        if (_handler == nullptr) return;
        
        // we no longer have to monitor the socket
        _handler->monitor(_connection, _socket, 0);
        
        // close the socket
        close(_socket);
    }
    
    /**
     *  The filedescriptor of this connection
     *  @return int
     */
    virtual int fileno() const override { return _socket; }
     
    /**
     *  Process the filedescriptor in the object    
     *  @param  fd          The filedescriptor that is active
     *  @param  flags       AMQP::readable and/or AMQP::writable
     *  @return             New implementation object
     */
    virtual TcpState *process(int fd, int flags)
    {
        // the socket must be the one this connection writes to
        if (fd != _socket) return this;
        
        // are we busy with sending or receiving data?
        if (_state == state_sending)
        {
            // try to send more data from the outgoing buffer
            auto result = _out.sendto(_ssl);
            
            // if this is a success, we can proceed with the event loop
            if (result > 0) return proceed();
            
            // the operation failed, we may have to repeat our call
            else return repeat(result);
        }
        else
        {
            // read data from ssl into the buffer
            auto result = _in.receivefrom(_ssl, _connection->expected());

            // if this is a success, we may have to update the monitor
            if (result > 0) return parse(result);
            
            // the operation failed, we may have to repeat our call
            else return repeat(result);
        }
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
            // try to send more data from the outgoing buffer
            auto result = _out.sendto(_ssl);
            
            // go to the next state
            auto *state = result > 0 ? proceed() : repeat(result);
            
            return state;
            
//            if (result > 0) return proceed();
//            
//            // the operation failed, we may have to repeat our call
//            else return repeat(result);
        }
        
        
        return this;
    }

    /**
     *  Send data over the connection
     *  @param  buffer      buffer to send
     *  @param  size        size of the buffer
     */
    virtual void send(const char *buffer, size_t size)
    {
        // put the data in the outgoing buffer
        _out.add(buffer, size);
        
        // if we're already busy with sending or receiving, we first have to wait
        // for that operation to complete before we can move on
        if (_state != state_idle) return;
        
        // object is now busy sending
        _state = state_sending;
        
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
     *  Report to the handler that the connection was nicely closed
     */
    virtual void reportClosed() override
    {
        // remember that the object is closed
        _closed = true;
        
        // if the previous operation is still in progress
        if (_state != state_idle) return;
        
        // wait until the connection is writable
        _handler->monitor(_connection, _socket, writable);
    }
}; 

/**
 *  End of namespace
 */
}
 
