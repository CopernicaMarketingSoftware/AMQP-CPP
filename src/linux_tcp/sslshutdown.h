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
     *  Proceed with the next operation after the previous operation was
     *  a success, possibly changing the filedescriptor-monitor
     *  @return TcpState*
     */
    TcpState *proceed()
    {
        // construct monitor to prevent that we access members if object is destructed
        Monitor monitor(this);
        
        // we're no longer interested in events
        _handler->monitor(_connection, _socket, 0);
        
        // stop if object was destructed
        if (!monitor) return nullptr;
    
        // close the socket
        close(_socket);
        
        // forget the socket
        _socket = -1;
        
        // go to the closed state
        return new TcpClosed(_connection, _handler);
    }
        
    /**
     *  Method to repeat the previous call
     *  @param  result      result of an earlier openssl operation
     *  @return TcpState*
     */
    TcpState *repeat(int result)
    {
        // error was returned, so we must investigate what is going on
        auto error = SSL_get_error(_ssl, result);
                        
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

            // @todo check how to handle this
            return this;
        }
    }
    

public:
    /**
     *  Constructor
     *  @param  connection  Parent TCP connection object
     *  @param  socket      The socket filedescriptor
     *  @param  ssl         The SSL structure
     *  @param  handler     User-supplied handler object
     */
    SslShutdown(TcpConnection *connection, int socket, const SslWrapper &ssl, TcpHandler *handler) : 
        TcpState(connection, handler),
        _ssl(ssl),
        _socket(socket)
    {
        // tell the handler to monitor the socket if there is an out
        _handler->monitor(_connection, _socket, readable); 
    }   
    
    /**
     * Destructor
     */
    virtual ~SslShutdown() noexcept
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
        
        // because the object might soon be destructed, we create a monitor to check this
        Monitor monitor(this);

        // close the connection
        auto result = SSL_shutdown(_ssl);
            
        // if this is a success, we can proceed with the event loop
        if (result > 0) return proceed();
            
        // the operation failed, we may have to repeat our call
        else return repeat(result);
    }
};

/**
 *  End of namespace
 */
}
