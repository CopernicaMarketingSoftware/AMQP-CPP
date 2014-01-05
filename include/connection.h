/**
 *  Class describing a mid-level Amqp connection
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
     *  Destructor
     */
    virtual ~Connection() {}

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
    size_t parse(char *buffer, size_t size)
    {
        return _implementation.parse(buffer, size);
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
     *  Some classes have access to private properties
     */
    friend class ChannelImpl;
};

/**
 *  End of namespace
 */
}

