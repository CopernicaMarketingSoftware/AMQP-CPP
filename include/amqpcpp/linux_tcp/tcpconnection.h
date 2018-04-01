/**
 *  TcpConnection.h
 *
 *  Extended Connection object that creates a TCP connection for the
 *  IO between the client application and the RabbitMQ server.
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
 *  Forward declarations
 */
class TcpState;

/**
 *  Class definition
 */
class TcpConnection :
    private ConnectionHandler,
    private Watchable
{
private:
    /**
     *  The state of the TCP connection - this state objecs changes based on 
     *  the state of the connection (resolving, connected or closed)
     *  @var    std::unique_ptr<TcpState>
     */
    std::shared_ptr<TcpState> _state;

    /**
     *  The underlying AMQP connection
     *  @var    Connection
     */
    Connection _connection;

    /**
     *  Method that is called when the heartbeat frequency is negotiated.
     *  @param  connection      The connection that suggested a heartbeat interval
     *  @param  interval        The suggested interval from the server
     *  @return uint16_t        The interval to use
     */
    virtual uint16_t onNegotiate(Connection *connection, uint16_t interval) override;

    /**
     *  Method that is called by the connection when data needs to be sent over the network
     *  @param  connection      The connection that created this output
     *  @param  buffer          Data to send
     *  @param  size            Size of the buffer
     */
    virtual void onData(Connection *connection, const char *buffer, size_t size) override;

    /**
     *  Method that is called when the server sends a heartbeat to the client
     *  @param  connection      The connection over which the heartbeat was received
     */
    virtual void onHeartbeat(Connection *connection) override;

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

    /**
     *  Parse a buffer that was received
     *  @param  buffer
     */
    uint64_t parse(const Buffer &buffer)
    {
        // pass to the AMQP connection
        return _connection.parse(buffer);
    }

    /**
     *  Classes that have access to private data
     */
    friend class SslConnected;
    friend class TcpConnected;
    friend class TcpChannel;

    
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
    virtual ~TcpConnection() noexcept;

    /**
     *  The filedescriptor that is used for this connection
     *  @return int
     */
    int fileno() const;

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
    
    /**
     *  Flush the connection - all unsent bytes are sent to the socket rigth away
     *  This is a blocking operation. The connection object normally only sends data
     *  when the socket is known to be writable, but with this method you can force
     *  the outgoing buffer to be fushed
     */
    void flush();
    
    /**
     *  Close the connection
     *  This closes all channels and the TCP connection
     *  @return bool
     */
    bool close()
    {
        // pass to the underlying connection
        return _connection.close();
    }

    /**
     *  The max frame size. Useful if you set up a buffer to parse incoming data: it does not have to exceed this size.
     *  @return uint32_t
     */
    uint32_t maxFrame() const
    {
        return _connection.maxFrame();
    }

    /**
     *  The number of bytes that can best be passed to the next call to the parse() method.
     *  @return uint32_t
     */
    uint32_t expected() const
    {
        return _connection.expected();
    }

    /**
      *  Return the number of channels this connection has.
      *  @return std::size_t
      */
    std::size_t channels() const
    {
        // return the amount of channels this connection has
        return _connection.channels();
    }

    /**
     *  The number of outgoing bytes queued on this connection.
     *  @return std::size_t
     */
    std::size_t queued() const;
    
    /**
     *  Send a heartbeat
     *  @return bool
     */
    bool heartbeat()
    {
        return _connection.heartbeat();
    }
};

/**
 *  End of namespace
 */
}
