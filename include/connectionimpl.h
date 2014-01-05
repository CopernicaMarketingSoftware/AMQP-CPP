/**
 *  Connection implementation
 *
 *  This is the implementation of the connection - a class that can only be
 *  constructed by the connection class itselves and that has all sorts of
 *  methods that are only useful inside the library
 * 
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class ConnectionImpl : public Watchable
{
protected:
    /**
     *  The parent connection object
     *  @var    Connection
     */
    Connection *_parent;

    /**
     *  The connection handler
     *  @var    ConnectionHandler
     */
    ConnectionHandler *_handler;

    /**
     *  State of the connection
     *  The current state is the last frame sent to the server
     *  @var    enum
     */
    enum {
        state_invalid,              // object is in an invalid state
        state_protocol,             // protocol headers are being passed
        state_handshake,            // busy with the handshake to open the connection
        state_connected,            // connection is set up and ready for communication         
        state_closed                // connection is closed
    } _state = state_protocol;

    /**
     *  All channels that are active
     *  @var    map
     */
    std::map<uint16_t, ChannelImpl*> _channels;

    /**
     *  The last unused channel ID
     *  @var    uint16_t
     */
    uint16_t _nextFreeChannel = 1;
    
    /**
     *  Max number of channels (0 for unlimited)
     *  @var    uint16_t
     */
    uint16_t _maxChannels = 0;
    
    /**
     *  Max frame size
     *  @var    uint32_t
     */
    uint32_t _maxFrame = 10000;

    /**
     *  The login for the server (login, password)
     *  @var    Login
     */
    Login _login;
    
    /**
     *  Vhost to connect to
     *  @var    string
     */
    std::string _vhost;
    
    /**
     *  Queued messages that should be sent after the connection has been established
     *  @var    queue
     */
    std::queue<OutBuffer> _queue;


private:
    /**
     *  Construct an AMQP object based on full login data
     * 
     *  The first parameter is a handler object. This handler class is
     *  an interface that should be implemented by the caller.
     * 
     *  Note that the constructor is private to ensure that nobody can construct
     *  this class, only the real Connection class via a friend construct
     * 
     *  @param  parent          Parent connection object
     *  @param  handler         Connection handler
     *  @param  login           Login data
     */
    ConnectionImpl(Connection *parent, ConnectionHandler *handler, const Login &login, const std::string &vhost);

public:
    /**
     *  Destructor
     */
    virtual ~ConnectionImpl();

    /**
     *  What is the state of the connection - is the protocol handshake completed?
     *  @return bool
     */
    bool protocolOk()
    {
        // must be busy doing the connection handshake, or already connected
        return _state == state_handshake || _state == state_connected;
    }
    
    /**
     *  Mark the protocol as being ok
     */
    void setProtocolOk()
    {
        // move on to handshake state
        if (_state == state_protocol) _state = state_handshake;
    }
    
    /**
     *  Are we fully connected?
     *  @return bool
     */
    bool connected()
    {
        // state must be connected
        return _state == state_connected;
    }
    
    /**
     *  Mark the connection as connected
     */
    void setConnected();
    
    /**
     *  Retrieve the login data
     *  @return Login
     */
    Login &login()
    {
        return _login;
    }
    
    /**
     *  Retrieve the vhost
     *  @return string
     */
    std::string &vhost()
    {
        return _vhost;
    }

    /**
     *  Store the max number of channels and max number of frames
     *  @param  channels    max number of channels
     *  @param  size        max frame size
     */
    void setCapacity(uint16_t channels, uint32_t size)
    {
        _maxChannels = channels;
        _maxFrame = size;
    }
    
    /**
     *  The max frame size
     *  @return uint32_t
     */
    uint32_t maxFrame()
    {
        return _maxFrame;
    }
    
    /**
     *  The max payload size for body frames
     *  @return uint32_t
     */
    uint32_t maxPayload()
    {
        // 8 bytes for header and end-of-frame byte
        return _maxFrame - 8;
    }
    
    /**
     *  Add a channel to the connection, and return the channel ID that it
     *  is allowed to use, or 0 when no more ID's are available
     *  @param  channel
     *  @return uint16_t
     */
    uint16_t add(ChannelImpl *channel);
    
    /**
     *  Remove a channel
     *  @param  channel
     */
    void remove(ChannelImpl *channel);
    
    /**
     *  Parse the buffer into a recognized frame
     *  
     *  Every time that data comes in on the connection, you should call this method to parse
     *  the incoming data, and let it handle by the AMQP library. This method returns the number
     *  of bytes that were processed.
     *
     *  If not all bytes could be processed because it only contained a partial frame, you should
     *  call this same method later on when more data is available. The AMQP library does not do
     *  any buffering, so it is up to the caller to ensure that the old data is also passed in that
     *  later call.
     *
     *  @param  buffer      buffer to decode
     *  @param  size        size of the buffer to decode
     *  @return             number of bytes that were processed
     */
    size_t parse(char *buffer, size_t size);

    /**
     *  Close the connection
     *  This will close all channels
     *  @return bool
     */
    bool close();

    /**
     *  Send a frame over the connection
     * 
     *  This is an internal method that you normally do not have to call yourself
     * 
     *  @param  frame       the frame to send
     *  @return             number of bytes sent
     */
    size_t send(const Frame &frame);

    /**
     *  Get a channel by its identifier
     * 
     *  This method only works if you had already created the channel before.
     *  This is an internal method that you will not need if you cache the channel
     *  object.
     * 
     *  @param  number          channel identifier
     *  @return channel         the channel object, or nullptr if not yet created
     */
    ChannelImpl *channel(int number)
    {
        auto iter = _channels.find(number);
        return iter == _channels.end() ? nullptr : iter->second;
    }

    /**
     *  Report an error message
     *  @param  message
     */
    void reportError(const std::string &message)
    {
        // close everything
        //  @todo is this not duplicate?
        close();

        // set connection state to closed
        _state = state_closed;
        
        // inform handler
        _handler->onError(_parent, message);

        // @Todo: notify all channels of closed connection
    }

    /**
     *  Set the Quality of Service (QOS) of the entire connection
     *  @param  prefetchSize        maximum size (in octets) of messages to be prefetched
     *  @param  prefetchCount       maximum number of messages to prefetch
     *  @return bool                whether the Qos frame is sent.
     */
    bool setQos(uint32_t prefetchSize, uint16_t prefetchCount);

    /**
     *  The actual connection is a friend and can construct this class
     */
    friend class Connection;


};

/**
 *  End of namespace
 */
}

