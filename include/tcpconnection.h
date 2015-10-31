/**
 *  TcpConnection.h
 *
 *  Extended Connection object that creates a TCP connection for the
 *  IO between the client application and the RabbitMQ server.
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
 *  Forward declarations
 */
class TcpState;

/**
 *  Class definition
 */
class TcpConnection : public Connection, private ConnectionHandler
{
private:
    /**
     *  The state of the TCP connection - this state objecs changes based on 
     *  the state of the connection (resolving, connected or closed)
     * 
     *  @var    TcpState
     */
    TcpState *_state = nullptr;
    

    /**
     *  Method that is called by the connection when data needs to be sent over the network
     *  @param  connection      The connection that created this output
     *  @param  buffer          Data to send
     *  @param  size            Size of the buffer
     */
    virtual void onData(Connection *connection, const char *buffer, size_t size) override;

    /**
     *  Method called when the connection ends up in an error state
     *  @param  connection      The connection that entered the error state
     *  @param  message         Error message
     */
    virtual void onError(Connection *connection, const char *message) override;

    /**
     *  Method that is called when the connection is established
     *  @param  connection      The connection that can now be used
     */
    virtual void onConnected(Connection *connection) override;

    /**
     *  Method that is called when the connection was closed.
     *  @param  connection      The connection that was closed and that is now unusable
     */
    virtual void onClosed(Connection *connection) override;

    
public:
    /**
     *  Constructor
     *  @param  handler         User implemented handler object
     *  @param  hostname        The address to connect to
     */
    TcpConnection(TcpHandler *handler, const Address &address);
    
    /**
     *  Destructor
     */
    virtual ~TcpConnection();

    /**
     *  Process the TCP connection
     * 
     *  This method should be called when the filedescriptor that is registered
     *  in the event loop becomes active. You should pass in a flag holding the
     *  flags AMQP::readable or AMQP::writable to indicate whether the descriptor
     *  was readable or writable, or bitwise-or if it was both
     * 
     *  @param  fd              The filedescriptor that became readable or writable
     *  @param  events          What sort of events occured?
     */
    void process(int fd, int flags);
};

/**
 *  End of namespace
 */
}
