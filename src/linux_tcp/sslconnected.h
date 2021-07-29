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
#include "sslwrapper.h"
#include "sslshutdown.h"
#include "sslerrorprinter.h"
#include <iostream>

/**
 * Set up namespace
 */
namespace AMQP {

/** 
 * Class definition
 */ 
class SslConnected : public TcpExtState 
{
private:
    /**
     *  The SSL structure
     *  @var SslWrapper
     */
    SslWrapper _ssl;

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
     *  @var int
     */
    int _state = SSL_ERROR_NONE;

    static const char *statetostring(int state)
    {
        switch (state) {
        case SSL_ERROR_NONE: return "SSL_ERROR_NONE";
        case SSL_ERROR_WANT_READ: return "SSL_ERROR_WANT_READ";
        case SSL_ERROR_WANT_WRITE: return "SSL_ERROR_WANT_WRITE";
        default: return "ERROR!";
        }
    }
    
    /**
     *  Should we close the connection after we've finished all operations?
     *  @var bool
     */
    bool _closed = false;
    
    /**
     *  Cached reallocation instruction
     *  @var size_t
     */
    size_t _reallocate = 0;
    

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
            _parent->onIdle(this, _socket, readable | writable);
        }
        else if (_closed)
        {
            // start the state that closes the connection
            return new SslShutdown(this, std::move(_ssl));
        }
        else
        {
            // let's wait until the socket becomes readable
            _parent->onIdle(this, _socket, readable);
        }
        
        // done
        return this;
    }
    
    /**
     *  Method to repeat the previous call
     *  @param  monitor     monitor to check if connection object still exists
     *  @return TcpState*
     */
    TcpState *repeat(const Monitor &monitor)
    {
        // if we are not able to repeat the call, we are in an error state and should tear down the connection
        if (!repeat()) return monitor.valid() ? new TcpClosed(this) : nullptr;

        // if the socket was closed in the meantime and we are not sending anything any more, we should initialize the shutdown sequence
        if (_closed && _state == SSL_ERROR_NONE) return new SslShutdown(this, std::move(_ssl));
        
        // otherwise, we just continue as we were, since the calls should be repeated in the future
        else return this;
    }

    /**
     *  Method to repeat the previous call, which has then been
     *  @return bool
     */
    bool repeat()
    {
        // check the error
        switch (_state) {
        case SSL_ERROR_WANT_READ:

            // allow chaining
            return true;

        case SSL_ERROR_WANT_WRITE:

            // reset state: we can determine whether we want to do pending writes via _out
            _state = SSL_ERROR_NONE;

            // we are done
            return true;

        // this case doesn't actually happen when repeat is called, since it will only be returned when
        // the result > 0 and therefore there is no error. it is here just to be sure.
        case SSL_ERROR_NONE:

            // nothing is wrong, we are done
            return true;

        default:
            {
                // get a human-readable error string
                const SslErrorPrinter message{_state};

                // report an error to user-space
                _parent->onError(this, message.data());
            }

            // ssl level error, we have to tear down the tcp connection
            return false;
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
        auto processed = _parent->onReceived(this, buffer);
        
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
     *  @return TcpState*
     */
    TcpState *write(const Monitor &monitor)
    {
        // assume default state
        _state = SSL_ERROR_NONE;

        // we are going to check for errors after the openssl operations, so we make
        // sure that the error queue is currently completely empty
        OpenSSL::ERR_clear_error();
        
        // because the output buffer contains a lot of small buffers, we can do multiple 
        // operations till the buffer is empty (but only if the socket is not also
        // readable, because then we want to read that data first instead of endless writes
        while (_out)
        {
            // try to send more data from the outgoing buffer
            auto result = _out.sendto(_ssl);
            
            // we may have to repeat the operation on failure
            if (result > 0) continue;
            
            // check for error
            _state = OpenSSL::SSL_get_error(_ssl, result);

            // the operation failed, we may have to repeat our call
            return repeat(monitor);
        }
        
        // proceed with the read operation or the event loop
        return proceed();
    }

    /**
     *  Perform a receive operation
     *  @param  monitor         object to check the existance of the connection object
     *  @return TcpState
     */
    TcpState *receive(const Monitor &monitor)
    {
        // assume default state
        _state = SSL_ERROR_NONE;

        // we are going to check for errors after the openssl operations, so we make 
        // sure that the error queue is currently completely empty
        OpenSSL::ERR_clear_error();

        // start a loop
        for (size_t expected = _parent->expected(); expected > 0; expected = _parent->expected())
        {
            // read data from ssl into the buffer
            auto result = _in.receivefrom(_ssl, expected);
            
            // if this is a failure, we are going to repeat the operation
            if (result <= 0)
            {
                _state = OpenSSL::SSL_get_error(_ssl, result);

                return repeat(monitor);
            }

            // go process the received data
            auto *nextstate = parse(monitor, result);
            
            // leap out if we moved to a different state
            if (nextstate != this) return nextstate;
        }
        
        // proceed with the write operation or the event loop
        return proceed();
    }
    
public:
    /**
     *  Constructor
     *  @param  state       The previous state
     *  @param  ssl         The SSL structure
     *  @param  buffer      The buffer that was already built
     */
    SslConnected(TcpExtState *state, SslWrapper &&ssl, TcpOutBuffer &&buffer) : 
        TcpExtState(state),
        _ssl(std::move(ssl)),
        _out(std::move(buffer)),
        _in(4096)
    {
        // tell the handler to monitor the socket if there is an out
        _state = SSL_ERROR_NONE;
        _parent->onIdle(this, _socket, readable | writable);
    }
    
    /**
     *  Destructor
     */
    virtual ~SslConnected() noexcept = default;
    
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

        TcpState *nextstate = this;

        if (_closed) nextstate = new SslShutdown(this, std::move(_ssl));

        if (nextstate == this && (flags & readable))
        {
            switch (_state) {
            case SSL_ERROR_NONE:
            case SSL_ERROR_WANT_READ:
                nextstate = receive(monitor);
                break;
            default:
                nextstate = new TcpClosed(this);
                break;
            }
        }
        if (nextstate == this /*&& (flags & writable)*/)
        {
            switch (_state) {
            case SSL_ERROR_NONE:
            case SSL_ERROR_WANT_READ:
                if (_out) nextstate = write(monitor);
                break;
            default:
                nextstate = new TcpClosed(this);
                break;
            }
        }

        return nextstate;
    }

    /**
     *  Send data over the connection
     *  @param  buffer      buffer to send
     *  @param  size        size of the buffer
     */
    virtual void send(const char *buffer, size_t size) override
    {
        // do nothing if already busy closing
        if (_closed) return;
        
        // if we're not idle, we can just add bytes to the buffer and we're done
        if (_out) return _out.add(buffer, size);

        // clear ssl-level error
        OpenSSL::ERR_clear_error();

        // get the result
        int result = OpenSSL::SSL_write(_ssl, buffer, size);  

        // if the result is larger than zero, we are successful
        if (result > 0) return; 
            
        // check for error
        _state = OpenSSL::SSL_get_error(_ssl, result);

        // put the data in the outgoing buffer
        _out.add(buffer, size);

        // the operation failed, we may have to repeat our call. this may detect that
        // ssl is in an error state, however that is ok because it will set an internal 
        // state to the error state so that on the next calls to state-changing objects, 
        // the tcp socket will be torn down
        if (repeat()) return;
    }

    /**
     *  Gracefully close the connection
     */
    virtual void close() override 
    { 
        // remember that the object is going to be closed
        _closed = true;
        
        // if the previous operation is still in progress we can wait for that
        if (_state != SSL_ERROR_NONE || _out) return;
        
        // let's wait until the socket becomes writable (because then we can start the shutdown)
        _parent->onIdle(this, _socket, readable | writable);
    }

    /**
     *  Install max-frame size
     *  @param  heartbeat   suggested heartbeat
     */
    virtual void maxframe(size_t maxframe) override
    {
        // remember that we have to reallocate (_in member can not be accessed because it is moved away)
        _reallocate = maxframe;
    }
}; 

/**
 *  End of namespace
 */
}
