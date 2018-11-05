/**
 *  TcpConnection.cpp
 *
 *  Implementation file for the TCP connection
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 - 2018 Copernica BV
 */

/**
 *  Dependencies
 */
#include "includes.h"
#include "tcpresolver.h"
#include "tcpstate.h"

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Constructor
 *  @param  handler         User implemented handler object
 *  @param  hostname        The address to connect to
 */
TcpConnection::TcpConnection(TcpHandler *handler, const Address &address) :
    _handler(handler),
    _state(new TcpResolver(this, address.hostname(), address.port(), address.secure())),
    _connection(this, address.login(), address.vhost()) {}

/**
 *  Destructor
 */
TcpConnection::~TcpConnection() noexcept = default;

/**
 *  The filedescriptor that is used for this connection
 *  @return int
 */
int TcpConnection::fileno() const
{
    // pass on to the state object
    return _state->fileno();
}

/**
 *  The number of outgoing bytes queued on this connection.
 *  @return std::size_t
 */
std::size_t TcpConnection::queued() const
{
    return _state->queued();
}

/**
 *  Process the TCP connection
 *  This method should be called when the filedescriptor that is registered
 *  in the event loop becomes active. You should pass in a flag holding the
 *  flags AMQP::readable or AMQP::writable to indicate whether the descriptor
 *  was readable or writable, or bitwise-or if it was both
 *  @param  fd              The filedescriptor that became readable or writable
 *  @param  events          What sort of events occured?
 */
void TcpConnection::process(int fd, int flags)
{
    // monitor the object for destruction, because you never know what the user
    Monitor monitor(this);
        
    // store the old state
    auto *oldstate = _state.get();

    // pass on the the state, that returns a new impl
    auto *newstate = _state->process(monitor, fd, flags);

    // if the state did not change, we do not have to update a member,
    // when the newstate is nullptr, the object is (being) destructed
    // and we do not have to do anything else either
    if (oldstate == newstate || newstate == nullptr) return;

    // in a bizarre set of circumstances, the user may have implemented the
    // handler in such a way that the connection object was destructed
    if (!monitor.valid()) 
    {
        // ok, user code is weird, connection object no longer exist, get rid of the state too
        delete newstate;
    }
    else
    {
        // replace it with the new implementation
        // @todo destructing the existing _state may destruct the entire object
        _state.reset(newstate);
    }
}

/**
 *  Flush the tcp connection
 */
void TcpConnection::flush()
{
    // monitor the object for destruction
    Monitor monitor(this);

    // keep looping
    while (true)
    {
        // flush the object
        auto *newstate = _state->flush(monitor);
        
        // done if object no longer exists
        if (!monitor.valid()) return;
        
        // also done if the object is still in the same state
        if (newstate == _state.get()) return;
        
        // replace the new state
        _state.reset(newstate);
    }
}

/**
 *  Close the connection
 *  @return bool
 */
bool TcpConnection::close(bool immediate)
{
    // @todo what if not yet connected / still doing a lookup / already disconnected?
    
    // if no immediate disconnect is needed, we can simply start the closing handshake
    if (!immediate) return _connection.close();
    
    // fail the connection / report the error to user-space
    _connection.fail("connection prematurely closed by client");
    
    // change the state
    _state.reset(new TcpClosed(_state.get()));
    
    // done, we return true because the connection is closed
    return true;
}

/**
 *  Method that is called when the heartbeat frequency is negotiated.
 *  @param  connection      The connection that suggested a heartbeat interval
 *  @param  interval        The suggested interval from the server
 *  @return uint16_t        The interval to use
 */
uint16_t TcpConnection::onNegotiate(Connection *connection, uint16_t interval)
{
    // tell the max-frame size
    _state->maxframe(connection->maxFrame());
    
    // tell the handler
    return _handler->onNegotiate(this, interval);
}

/**
 *  Method that is called by the connection when data needs to be sent over the network
 *  @param  connection      The connection that created this output
 *  @param  buffer          Data to send
 *  @param  size            Size of the buffer
 */
void TcpConnection::onData(Connection *connection, const char *buffer, size_t size)
{
    // send the data over the connection
    _state->send(buffer, size);
}

/**
 *  Method called when the connection ends up in an error state
 *  @param  connection      The connection that entered the error state
 *  @param  message         Error message
 */
void TcpConnection::onError(Connection *connection, const char *message)
{
    // monitor to check if "this" is destructed
    Monitor monitor(this);
    
    // remember the old state (this is necessary because _state may be modified by user-code)
    auto *oldstate = _state.get();
    
    // tell the state that the connection should be closed asap
    auto *newstate = _state->close();
    
    // leap out if nothing changes
    if (newstate == nullptr || newstate == oldstate) return;
    
    // assign the new state
    _state.reset(newstate);
}

/**
 *  Method that is called when the connection was closed.
 *  @param  connection      The connection that was closed and that is now unusable
 */
void TcpConnection::onClosed(Connection *connection)
{
    // monitor to check if "this" is destructed
    Monitor monitor(this);

    // remember the old state (this is necessary because _state may be modified by user-code)
    auto *oldstate = _state.get();
    
    // tell the state that the connection should be closed asap
    auto *newstate = _state->close();
    
    // leap out if nothing changes
    if (newstate == nullptr || newstate == oldstate) return;
    
    // assign the new state
    _state.reset(newstate);
}

/**
 *  End of namespace
 */
}

