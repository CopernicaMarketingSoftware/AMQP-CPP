/**
 *  Test program for the AMQP library
 *
 *  @documentation private
 */

/**
 *  Callback function that should be implemented by the calling application
 * 
 *  The frame should be deallocated by calling amqp_frame_destroy() after you've
 *  finished processing it.
 * 
 *  @param  frame       The received frame
 *  @param  cookie      Pointer to cookie data
 */
static void my_callback(amqp_frame_t *frame, void *cookie)
{
    AmqpHandler *handler = (AmqpHandler *)cookie;
    
    // @todo process frame
    
}

/**
 *  Class that handles IO for the AMQP socket
 */
class AmqpHandler : private Network::TcpHandler
{
private:
    /**
     *  The TCP socket
     *  @var    Network::TcpSocket
     */
    Network::TcpSocket _socket;

    


    /**
     *  Method that is called when the connection failed
     *  @param  socket      Pointer to the socket
     */
    virtual void onFailure(TcpSocket *socket) 
    {
        exit();
    }
    
    /**
     *  Method that is called when the connection timed out (which also is a failure
     *  @param  socket      Pointer to the socket
     */
    virtual void onTimeout(TcpSocket *socket) 
    {
        exit();
    }
    
    /**
     *  Method that is called when the connection succeeded
     *  @param  socket      Pointer to the socket
     */
    virtual void onConnected(TcpSocket *socket)
    {
        // eerste frame gaan versturen - of wachten op eerste frame
        amqp_send_handshake(_handle, "1.0");
    }
    
    /**
     *  Method that is called when the socket is closed (as a result of a TcpSocket::close() call)
     *  @param  socket      Pointer to the socket
     */
    virtual void onClosed(TcpSocket *socket) 
    {
        exit();
    }

    /**
     *  Method that is called when the peer closed the connection
     *  @param  socket      Pointer to the socket
     */
    virtual void onLost(TcpSocket *socket) 
    {
        exit();
    }
    
    /**
     *  Method that is called when data is received on the socket
     *  @param  socket      Pointer to the socket
     *  @param  buffer      Pointer to the fill input buffer
     */
    virtual void onData(TcpSocket *socket, Buffer *buffer) 
    {
        ssize bytes = amqp_parse_frames(_handle, buffer->data(), buffer->size());
        
        if (bytes > 0) buffer->shrink(bytes);
        
    }

public:
    /**
     *  Constructor
     */
    AmqpHandler() : _socket(Event::MainLoop::instance(), this)
    {
        _handler = amqp_create_frame_handler(parse_frame, this);

        _handle = amqp_create_message_handler(
    }
    
    /**
     *  Destructor
     */
    virtual ~AmqpHandler() 
    {
        amqp_destroy_frame_parser(_handle);
    }
};

/**
 *  Main procedure
 *  @param  argc
 *  @param  argv
 *  @return integer
 */
int main(int argc, const char *argv[])
{
    AmqpHandler handler;
    
    // run the application
    Event::MainLoop::instance()->run();
    
    return 0;
}

