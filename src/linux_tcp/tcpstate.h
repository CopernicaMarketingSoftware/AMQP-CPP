/**
 *  TcpState.h
 *
 *  Base class / interface of the various states of the TCP connection
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 - 2018 Copernica BV
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
     *  The filedescriptor of this connection
     *  @return int
     */
    virtual int fileno() const { return -1; }

    /**
     *  The number of outgoing bytes queued on this connection.
     *  @return size_t
     */
    virtual std::size_t queued() const { return 0; }

    /**
     *  Process the filedescriptor in the object
     * 
     *  This method should return the handler object that will be responsible for
     *  all future readable/writable events for the file descriptor, or nullptr
     *  if the underlying connection object has already been destructed by the
     *  user and it would be pointless to set up a new handler.
     * 
     *  @param  monitor     Monitor that can be used to check if the tcp connection is still alive
     *  @param  fd          The filedescriptor that is active
     *  @param  flags       AMQP::readable and/or AMQP::writable
     *  @return             New implementation object
     */
    virtual TcpState *process(const Monitor &monitor, int fd, int flags)
    {
        // default implementation does nothing and preserves same implementation
        return this;
    }
    
    /**
     *  Send data over the connection
     *  @param  buffer      Buffer to send
     *  @param  size        Size of the buffer
     */
    virtual void send(const char *buffer, size_t size)
    {
        // default does nothing
    }

    /**
     *  Flush the connection, all outgoing operations should be completed.
     * 
     *  If the state changes during the operation, the new state object should
     *  be returned instead, or nullptr if the user has closed the connection
     *  in the meantime. If the connection object got destructed by a user space
     *  call, this method should return nullptr. A monitor object is pass in to
     *  allow the flush() method to check if the connection still exists.
     * 
     *  If this object returns a new state object (instead of "this"), the 
     *  connection object will immediately proceed with calling flush() on that 
     *  new state object too.
     * 
     *  @param  monitor     Monitor that can be used to check if the tcp connection is still alive
     *  @return TcpState    New implementation object
     */
    virtual TcpState *flush(const Monitor &monitor) { return this; }

    /**
     *  Report to the handler that heartbeat negotiation is going on
     *  @param  heartbeat   suggested heartbeat
     *  @return uint16_t    accepted heartbeat
     */
    virtual uint16_t reportNegotiate(uint16_t heartbeat)
    {
        // pass to handler
        return _handler->onNegotiate(_connection, heartbeat);
    }
    
    /**
     *  Report to the handler that the object is in an error state.
     * 
     *  This is the last method to be called on the handler object, from now on
     *  the handler will no longer be called to report things to user space. 
     *  The state object itself stays active, and further calls to process()
     *  may be possible.
     * 
     *  @param  error
     */
    virtual void reportError(const char *error)
    {
        // pass to handler
        _handler->onError(_connection, error);
    }

    /**
     *  Report that a heartbeat frame was received
     */
    virtual void reportHeartbeat()
    {
        // pass to handler
        _handler->onHeartbeat(_connection);
    }
    
    /**
     *  Report to the handler that the connection is ready for use
     */
    virtual void reportConnected()
    {
        // pass to handler
        _handler->onConnected(_connection);
    }
    
    /**
     *  Report to the handler that the connection was correctly closed, after
     *  the user has called the Connection::close() method. The underlying TCP
     *  connection still has to be closed.
     * 
     *  This is the last method that is called on the object, from now on no
     *  other methods may be called on the _handler variable.
     */
    virtual void reportClosed()
    {
        // pass to handler
        _handler->onClosed(_connection);
    }
};

/**
 *  End of namespace
 */
}

