/**
 *  MyConnection.h
 *
 *  Our own test implementation for a connection handler
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Class definition
 */
class MyConnection : 
    public AMQP::ConnectionHandler,
    public Network::TcpHandler
{
private:
    /**
     *  The actual TCP socket that is connected with RabbitMQ
     *  @var    TcpSocket
     */
    Network::TcpSocket _socket;
    
    /**
     *  The AMQP connection
     *  @var    Connection
     */
    std::shared_ptr<AMQP::Connection> _connection;
    
    /**
     *  The AMQP channel
     *  @var    Channel
     */
    std::shared_ptr<AMQP::Channel> _channel;

    /**
     *  Method that is called when the connection failed
     *  @param  socket      Pointer to the socket
     */
    virtual void onFailure(Network::TcpSocket *socket) override;

    /**
     *  Method that is called when the connection timed out (which also is a failure
     *  @param  socket      Pointer to the socket
     */
    virtual void onTimeout(Network::TcpSocket *socket) override;
    
    /**
     *  Method that is called when the connection succeeded
     *  @param  socket      Pointer to the socket
     */
    virtual void onConnected(Network::TcpSocket *socket) override;
    
    /**
     *  Method that is called when the socket is closed (as a result of a TcpSocket::close() call)
     *  @param  socket      Pointer to the socket
     */
    virtual void onClosed(Network::TcpSocket *socket) override;

    /**
     *  Method that is called when the peer closed the connection
     *  @param  socket      Pointer to the socket
     */
    virtual void onLost(Network::TcpSocket *socket) override;
    
    /**
     *  Method that is called when data is received on the socket
     *  @param  socket      Pointer to the socket
     *  @param  buffer      Pointer to the fill input buffer
     */
    virtual void onData(Network::TcpSocket *socket, Network::Buffer *buffer) override;
    
    /**
     *  Method that is called when data needs to be sent over the network
     *
     *  Note that the AMQP library does no buffering by itself. This means 
     *  that this method should always send out all data or do the buffering
     *  itself.
     *
     *  @param  connection      The connection that created this output
     *  @param  buffer          Data to send
     *  @param  size            Size of the buffer
     */
    virtual void onData(AMQP::Connection *connection, const char *buffer, size_t size) override;
    
    /**
     *  When the connection ends up in an error state this method is called.
     *  This happens when data comes in that does not match the AMQP protocol
     *  
     *  After this method is called, the connection no longer is in a valid
     *  state and can be used. In normal circumstances this method is not called.
     *
     *  @param  connection      The connection that entered the error state
     *  @param  message         Error message
     */
    virtual void onError(AMQP::Connection *connection, const char *message) override;

    /**
     *  Method that is called when the login attempt succeeded. After this method
     *  was called, the connection is ready to use
     *
     *  @param  connection      The connection that can now be used
     */
    virtual void onConnected(AMQP::Connection *connection) override;

public:
    /**
     *  Constructor
     *  @param  ip
     */
    MyConnection(const std::string &ip);
    
    /**
     *  Destructor
     */
    virtual ~MyConnection();


};
