/**
 *  SslShutdown.h
 *
 *  Class that takes care of the final handshake to close a SSL connection
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2018 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Begin of namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class SslShutdown : public TcpState, private Watchable
{
private:
    /**
     *  The SSL context
     *  @var SslWrapper
     */
    SslWrapper _ssl;

    /** 
     *  Socket file descriptor
     *  @var int
     */
    int _socket;
    
    /**
     *  Have we already notified user space of connection end?
     *  @var bool
     */
    bool _finalized;


    /**
     *  Close the socket
     *  @return bool
     */
    bool close()
    {
        // skip if already closed
        if (_socket < 0) return false;
        
        // we're no longer interested in events
        _handler->monitor(_connection, _socket, 0);
        
        // close the socket
        ::close(_socket);
        
        // forget the socket
        _socket = -1;
        
        // done
        return true;
    }

    /**
     *  Report an error
     *  @param  monitor         object to check if connection still exists
     *  @return TcpState*
     */
    TcpState *reporterror(const Monitor &monitor)
    {
        // close the socket
        close();
        
        // if we have already told user space that connection is gone
        if (_finalized) return new TcpClosed(this);
        
        // object will be finalized now
        _finalized = true;
        
        // inform user space that the party is over
        _handler->onError(_connection, "ssl shutdown error");
        
        // go to the final state (if not yet disconnected)
        return monitor.valid() ? new TcpClosed(this) : nullptr;
    }

    /**
     *  Proceed with the next operation after the previous operation was
     *  a success, possibly changing the filedescriptor-monitor
     *  @param  monitor         object to check if connection still exists
     *  @return TcpState*
     */
    TcpState *proceed(const Monitor &monitor)
    {
        // close the socket
        close();
        
        // if we have already told user space that connection is gone
        if (_finalized) return new TcpClosed(this);
        
        // object will be finalized now
        _finalized = true;
        
        // inform user space that the party is over
        _handler->onClosed(_connection);
        
        // go to the final state (if not yet disconnected)
        return monitor.valid() ? new TcpClosed(this) : nullptr;
    }
        
    /**
     *  Method to repeat the previous call
     *  @param  monitor     object to check if connection still exists
     *  @param  result      result of an earlier openssl operation
     *  @return TcpState*
     */
    TcpState *repeat(const Monitor &monitor, int result)
    {
        // error was returned, so we must investigate what is going on
        auto error = OpenSSL::SSL_get_error(_ssl, result);

        // check the error
        switch (error) {
        case SSL_ERROR_WANT_READ:
            // the operation must be repeated when readable
            _handler->monitor(_connection, _socket, readable);
            return this;
        
        case SSL_ERROR_WANT_WRITE:
            // wait until socket becomes writable again
            _handler->monitor(_connection, _socket, readable | writable);
            return this;
            
        default:
            // go to the final state (if not yet disconnected)
            return reporterror(monitor);
        }
    }
    

public:
    /**
     *  Constructor
     *  @param  connection  Parent TCP connection object
     *  @param  socket      The socket filedescriptor
     *  @param  ssl         The SSL structure
     *  @param  finalized   Is the user already notified of connection end (onError() has been called)
     *  @param  handler     User-supplied handler object
     */
    SslShutdown(TcpConnection *connection, int socket, SslWrapper &&ssl, bool finalized, TcpHandler *handler) : 
        TcpState(connection, handler),
        _ssl(std::move(ssl)),
        _socket(socket),
        _finalized(finalized)
    {
        // wait until the socket is accessible
        _handler->monitor(_connection, _socket, readable | writable); 
    }   
    
    /**
     * Destructor
     */
    virtual ~SslShutdown() noexcept
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
     *  Process the filedescriptor in the object    
     *  @param  monitor     Object to check if connection still exists
     *  @param  fd          The filedescriptor that is active
     *  @param  flags       AMQP::readable and/or AMQP::writable
     *  @return             New implementation object
     */
    virtual TcpState *process(const Monitor &monitor, int fd, int flags) override
    {
        // the socket must be the one this connection writes to
        if (fd != _socket) return this;
        
        // close the connection
        auto result = OpenSSL::SSL_shutdown(_ssl);
        
        // on result==0 we need an additional call
        while (result == 0) result = OpenSSL::SSL_shutdown(_ssl);
        
        // if this is a success, we can proceed with the event loop
        if (result > 0) return proceed(monitor);
            
        // the operation failed, we may have to repeat our call
        else return repeat(monitor, result);
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

        // keep looping
        while (true)
        {
            // close the connection
            auto result = OpenSSL::SSL_shutdown(_ssl);

            // on result==0 we need an additional call
            while (result == 0) result = OpenSSL::SSL_shutdown(_ssl);
                
            // if this is a success, we can proceed with the event loop
            if (result > 0) return proceed(monitor);

            // error was returned, so we must investigate what is going on
            auto error = OpenSSL::SSL_get_error(_ssl, result);
            
            // check the error
            switch (error) {

            // if openssl reports that socket readability or writability is needed,
            // we wait for that until this situation is reached
            case SSL_ERROR_WANT_READ:   wait.readable(); break;
            case SSL_ERROR_WANT_WRITE:  wait.active(); break;
        
            // something is wrong, we proceed to the next state
            default: return reporterror(monitor);
            }
        }
    }
};

/**
 *  End of namespace
 */
}
