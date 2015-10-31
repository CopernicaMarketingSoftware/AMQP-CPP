/**
 *  TcpState.h
 *
 *  Base class / interface of the various states of the TCP connection
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class TcpState
{
protected:
    /**
     *  Parent TcpConnection object as is seen by the user
     *  @var TcpConnection
     */
    TcpConnection *_connection;

    /**
     *  User-supplied handler
     *  @var TcpHandler
     */
    TcpHandler *_handler;

protected:
    /**
     *  Protected constructor
     *  @param  connection  Original TCP connection object
     *  @param  handler     User-supplied handler class
     */
    TcpState(TcpConnection *connection, TcpHandler *handler) : 
        _connection(connection), _handler(handler) {}

    /**
     *  Protected "copy" constructor
     *  @param  state       Original TcpState object
     */
    TcpState(const TcpState *state) :
        _connection(state->_connection), _handler(state->_handler) {}

public:
    /**
     *  Virtual destructor
     */
    virtual ~TcpState() = default;

    /**
     *  Process the filedescriptor in the object    
     *  @param  fd          The filedescriptor that is active
     *  @param  flags       AMQP::readable and/or AMQP::writable
     *  @return             New implementation object
     */
    virtual TcpState *process(int fd, int flags)
    {
        // default implementation does nothing and preserves same implementation
        return this;
    }
    
    /**
     *  Send data over the connection
     *  @param  buffer      buffer to send
     *  @param  size        size of the buffer
     */
    virtual void send(const char *buffer, size_t size)
    {
        // default does nothing
    }
    
    /**
     *  Report to the handler that the object is in an error state
     *  @param  error
     */
    void reportError(const char *error)
    {
        // pass to handler
        _handler->onError(_connection, error);
    }
    
    /**
     *  Report to the handler that the connection is ready for use
     */
    void reportConnected()
    {
        // pass to handler
        _handler->onConnected(_connection);
    }
    
    /**
     *  Report to the handler that the connection was nicely closed
     */
    void reportClosed()
    {
        // pass to handler
        _handler->onClosed(_connection);
    }
};

/**
 *  End of namespace
 */
}

