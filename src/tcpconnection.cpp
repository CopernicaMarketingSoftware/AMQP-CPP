/**
 *  TcpConnection.cpp
 *
 *  Implementation file for the TCP connection
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Dependencies
 */
#include "includes.h"
#include "tcpresolver.h"

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
    _state(new TcpResolver(this, address.hostname(), address.port(), handler)),
    _connection(this, address.login(), address.vhost()) {}

/**
 *  Destructor
 */
TcpConnection::~TcpConnection()
{
    // remove the state object
    delete _state;
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
    // pass on the the state, that returns a new impl
    auto *result = _state->process(fd, flags);
    
    // skip if the same state is continued to be used, or when the process()
    // method returns nullptr (which only happens when the object is destructed,
    // and "this" is no longer valid)
    if (!result || result == _state) return;
    
    // remove old state
    delete _state;
    
    // replace it with the new implementation
    _state = result;
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
    // send the data over the connecction
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
    // current object is going to be removed, wrap it in a unique pointer to enforce that
    std::unique_ptr<TcpState> ptr(_state);
    
    // object is now in a closed state
    _state = new TcpClosed(_state);
    
    // tell the implementation to report the error
    ptr->reportError(message);
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
    // current object is going to be removed, wrap it in a unique pointer to enforce that
    std::unique_ptr<TcpState> ptr(_state);
    
    // object is now in a closed state
    _state = new TcpClosed(_state);
    
    // tell the implementation to report the error
    ptr->reportClosed();
}

/**
 *  End of namespace
 */
}

