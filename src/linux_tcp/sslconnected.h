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
#include "tcpstate.h"
#include "wait.h"
#include "sslwrapper.h"
#include "sslshutdown.h"

#include <amqpcpp/monitor.h>
#include <amqpcpp/flags.h>

#include <amqpcpp/linux_tcp/tcpconnection.h>
#include <amqpcpp/linux_tcp/tcphandler.h>

#include <cassert>

extern "C"
{
#include <unistd.h>
}

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
     *  Remember what the last logical SSL write operation that failed wanted from the socket
     *  @var int
     */
    int _write_want_flags;

    /**
     *  Remember what the last logical SSL read operation that failed wanted from the socket
     *  This is initialized to both readable and writable to ensure the first process() will
     *  try to read.
     *  @var int
     */
    int _receive_want_flags = readable | writable;

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

    void setup_monitor()
    {
        if (_write_want_flags | _receive_want_flags)
        {
            _handler->monitor(_connection, _socket, _write_want_flags | _receive_want_flags);
        }
    }

    /**
     *  Proceed with the next operation after the previous operation was
     *  a success, possibly changing the filedescriptor-monitor
     *  @return TcpState*
     */
    TcpState *proceed()
    {
        if (_closed && !_out)
        {
            // start the state that closes the connection
            auto *nextstate = new SslShutdown(_connection, _socket, std::move(_ssl), _finalized, _handler);

            // we forget the current socket to prevent that it gets destructed
            _socket = -1;

            // report the next state
            return nextstate;
        }

        // setup the monitor when our async operations didn't immediately complete
        setup_monitor();

        return this;
    }

    /**
     *  Method to repeat the previous call\
     *  @param  monitor     monitor to check if connection object still exists
     *  @param  result      result of an earlier SSL_get_error call
     *  @return TcpState*
     */
    TcpState *repeat(const Monitor &monitor, int &want_flags, int error)
    {
        // check the error
        switch (error) {
        case SSL_ERROR_WANT_READ:
            want_flags = readable;

            // allow chaining
            return monitor.valid() ? this : nullptr;

        case SSL_ERROR_WANT_WRITE:
            want_flags = writable;

            // allow chaining
            return monitor.valid() ? this : nullptr;

        case SSL_ERROR_NONE:
            // can not happen since repeat is only called when an SSL function failed
            assert(false);

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
     *  Write as much from _out as we can. This modifies _write_want_flags such that it is
     *  0 if everything was written or set to whatever the failed SSL_Write wanted from the socket
     *  @param  monitor         object to check the existence of the connection object
     *  @return TcpState*
     */
    TcpState *write(const Monitor &monitor)
    {
        // we are going to check for errors after the openssl operations, so we make
        // sure that the error queue is currently completely empty
        OpenSSL::ERR_clear_error();

        // because the output buffer contains a lot of small buffers, we can do multiple
        // operations till the buffer is empty (but only if the socket is not also
        // readable, because then we want to read that data first instead of endless writes
        do
        {
            // try to send more data from the outgoing buffer
            auto result = _out.sendto(_ssl);

            // we may have to repeat the operation on failure
            if (result > 0) continue;

            // check for error
            auto error = OpenSSL::SSL_get_error(_ssl, result);

            // the operation failed, we may have to repeat our call
            return repeat(monitor, _write_want_flags, error);
        }
        while (_out);

        // Doesn't matter
        _write_want_flags = 0;
        return monitor.valid() ? this : nullptr;
    }

    /**
     *  Read as much as we can. This modifies _receive_want_flags such that it is to whatever the
     *  failed SSL_Read wanted from the socket
     *  @param  monitor         object to check the existance of the connection object
     *  @return TcpState
     */
    TcpState *receive(const Monitor &monitor)
    {
        // we are going to check for errors after the openssl operations, so we make
        // sure that the error queue is currently completely empty
        OpenSSL::ERR_clear_error();

        do
        {
            // read data from ssl into the buffer
            auto result = _in.receivefrom(_ssl, _connection->expected());

            // if this is a failure, we are going to repeat the operation
            if (result <= 0)
            {
                return repeat(monitor, _receive_want_flags, OpenSSL::SSL_get_error(_ssl, result));
            }

            // go process the received data
            auto *nextstate = parse(monitor, result);

            // leap out if we moved to a different state
            if (nextstate != this) return nextstate;
        }
        while (1);
        // We **really** read until we fail so that we know for sure what SSL wants from the socket
        // otherwise we can't know whether we have to monitor for readable or writable
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
        _write_want_flags(_out ? readable | writable : 0)
    {
        setup_monitor();
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

        if (_out && (_write_want_flags & flags)) {
            TcpState *new_state = write(monitor);
            if (new_state != this) return new_state;
        }

        if (_receive_want_flags & flags) {
            TcpState *new_state = receive(monitor);
            if (new_state != this) return new_state;
        }
        return proceed();
    }

    /**
     *  Flush the connection, sent all buffered data to the socket
     *  @param  monitor     Object to check if connection still exists
     *  @return TcpState    new tcp state
     */
    virtual TcpState *flush(const Monitor &monitor) override
    {
        // create an object to wait for the filedescriptor to becomes active
        Wait wait(_socket);

        // we are going to check for errors after the openssl operations, so we make
        // sure that the error queue is currently completely empty
        OpenSSL::ERR_clear_error();

        // keep looping while we have an outgoing buffer
        while (_out)
        {
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
                auto *nextstate = repeat(monitor, _write_want_flags, error);

                // leap out if we move to a different state
                if (nextstate != this) return nextstate;

                // check the type of error, and wait now
                switch (error) {
                case SSL_ERROR_WANT_READ:   wait.readable(); break;
                case SSL_ERROR_WANT_WRITE:  wait.active(); break;
                }
            }
        }

        _write_want_flags = 0;

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

        // Try to immediately send the data (asynchronously) to make sure whe know what
        // event (readable/writable) we need to monitor for.
        Monitor monitor(this);
        write(monitor);

        // setup the monitor when our async operations didn't immediately complete
        setup_monitor();
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

        // setup the monitor when our async operations didn't immediately complete
        setup_monitor();
    }
};

/**
 *  End of namespace
 */
}
