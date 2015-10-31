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
 *  @param  address         AMQP address object
 */
TcpConnection::TcpConnection(TcpHandler *handler, const Address &address) : 
    Connection(this, address.login(), address.vhost()),
    _state(new TcpResolver(this, address.hostname(), address.port(), handler)) {}
    
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
    
    // skip if the same state is continued to be used
    if (result == _state) return;
    
    // remove old state
    delete _state;
    
    // replace it with the new implementation
    _state = result;
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

