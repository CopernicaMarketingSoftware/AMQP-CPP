/**
 *  Class describing a mid-level Amqp connection
 * 
 *  @copyright 2014 Copernica BV
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
class Connection
{
private:
    /**
     *  The actual implementation
     *  @var    ConnectionImpl
     */
    ConnectionImpl _implementation;

public:
    /**
     *  Construct an AMQP object based on full login data
     * 
     *  The first parameter is a handler object. This handler class is
     *  an interface that should be implemented by the caller.
     * 
     *  @param  handler         Connection handler
     *  @param  login           Login data
     *  @param  vhost           Vhost to use
     */
    Connection(ConnectionHandler *handler, const Login &login, const std::string &vhost) : _implementation(this, handler, login, vhost) {}

    /**
     *  Construct with default vhost
     *  @param  handler         Connection handler
     *  @param  login           Login data
     */
    Connection(ConnectionHandler *handler, const Login &login) : _implementation(this, handler, login, "/") {}

    /**
     *  Construct an AMQP object with default login data and default vhost
     *  @param  handler         Connection handler
     */
    Connection(ConnectionHandler *handler, const std::string &vhost) : _implementation(this, handler, Login(), vhost) {}

    /**
     *  Construct an AMQP object with default login data and default vhost
     *  @param  handler         Connection handler
     */
    Connection(ConnectionHandler *handler) : _implementation(this, handler, Login(), "/") {}

    /**
     *  No copy'ing, we do not support having two identical connection objects
     *  @param  connection
     */
    Connection(const Connection &connection) = delete;

    /**
     *  Destructor
     */
    virtual ~Connection() {}

    /**
     *  No assignments of other connections
     *  @param  connection
     *  @return Connection
     */
    Connection &operator=(const Connection &connection) = delete;
    
    /**
     *  Retrieve the login data
     *  @return Login
     */
    const Login &login() const
    {
        return _implementation.login();
    }

    /**
     *  Retrieve the vhost
     *  @return string
     */
    const std::string &vhost() const
    {
        return _implementation.vhost();
    }

    /**
     *  Parse data that was recevied from RabbitMQ
     *  
     *  Every time that data comes in from RabbitMQ, you should call this method to parse
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
    uint64_t parse(const char *buffer, size_t size)
    {
        return _implementation.parse(ByteBuffer(buffer, size));
    }

    /**
     *  Parse data that was recevied from RabbitMQ
     *  
     *  Every time that data comes in from RabbitMQ, you should call this method to parse
     *  the incoming data, and let it handle by the AMQP library. This method returns the number
     *  of bytes that were processed.
     *
     *  If not all bytes could be processed because it only contained a partial frame, you should
     *  call this same method later on when more data is available. The AMQP library does not do
     *  any buffering, so it is up to the caller to ensure that the old data is also passed in that
     *  later call.
     * 
     *  This method accepts a buffer object. This is an interface that is defined by the AMQP
     *  library, that can be implemented by you to allow faster access to a buffer.
     *
     *  @param  buffer      buffer to decode
     *  @return             number of bytes that were processed
     */
    uint64_t parse(const Buffer &buffer)
    {
        return _implementation.parse(buffer);
    }
    
    /**
     *  Max frame size
     *  
     *  If you allocate memory to receive data that you are going to pass to the parse() method,
     *  it might be useful to have an insight in the max frame size. The parse() method process
     *  one frame at a time, so you must at least be able to read in buffers of this specific
     *  frame size.
     * 
     *  @return size_t
     */
    uint32_t maxFrame() const
    {
        return _implementation.maxFrame();
    }
    
    /**
     *  Expected number of bytes for the next parse() call.
     * 
     *  This method returns the number of bytes that the next call to parse() at least expects to 
     *  do something meaningful with it.
     * 
     *  @return size_t
     */
    uint32_t expected() const
    {
        return _implementation.expected();
    }

    /**
     *  Close the connection
     *  This will close all channels
     *  @return bool
     */
    bool close()
    {
        return _implementation.close();
    }

    /**
     *  Retrieve the number of channels that are active for this connection
     *  @return std::size_t
     */
    std::size_t channels() const
    {
        return _implementation.channels();
    }
    
    /**
     *  Is the connection busy waiting for an answer from the server? (in the
     *  meantime you can already send more instructions over it)
     *  @return bool
     */
    std::size_t waiting() const
    {
        return _implementation.waiting();
    }

    /**
     *  Retrieve the heartbeat delay used by this connection
     *  @return uint16_t
     */
    uint16_t heartbeat() const
    {
        return _implementation.heartbeat();
    }

    /**
     *  Some classes have access to private properties
     */
    friend class ChannelImpl;
};

/**
 *  End of namespace
 */
}

