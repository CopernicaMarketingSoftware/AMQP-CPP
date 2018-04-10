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
    _state(new TcpResolver(this, address.hostname(), address.port(), address.secure(), handler)),
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

    // pass on the the state, that returns a new impl
    auto *result = _state->process(monitor, fd, flags);

    // are we still valid
    if (!monitor.valid()) return;

    // skip if the same state is continued to be used, or when the process()
    // method returns nullptr (which only happens when the object is destructed,
    // and "this" is no longer valid)
    if (!result || result == _state.get()) return;

    // replace it with the new implementation
    _state.reset(result);
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
 *  Method that is called when the heartbeat frequency is negotiated.
 *  @param  connection      The connection that suggested a heartbeat interval
 *  @param  interval        The suggested interval from the server
 *  @return uint16_t        The interval to use
 */
uint16_t TcpConnection::onNegotiate(Connection *connection, uint16_t interval)
{
    // the state object should do this
    return _state->reportNegotiate(interval);
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
 *  Method that is called when the server sends a heartbeat to the client
 *  @param  connection      The connection over which the heartbeat was received
 */
void TcpConnection::onHeartbeat(Connection *connection)
{
    // let the state object do this
    _state->reportHeartbeat();
}

/**
 *  Method called when the connection ends up in an error state
 *  @param  connection      The connection that entered the error state
 *  @param  message         Error message
 */
void TcpConnection::onError(Connection *connection, const char *message)
{
    // tell the implementation to report the error
    _state->reportError(message);
}

/**
 *  Method that is called when the connection is established
 *  @param  connection      The connection that can now be used
 */
void TcpConnection::onConnected(Connection *connection)
{
    // tell the implementation to report the status
    _state->reportConnected();
}

/**
 *  Method that is called when the connection was closed.
 *  @param  connection      The connection that was closed and that is now unusable
 */
void TcpConnection::onClosed(Connection *connection)
{
    // tell the implementation to report that connection is closed now
    _state->reportClosed();
}

/**
 *  End of namespace
 */
}

