/**
 *  ConnectionHandler.h
 *
 *  Interface that should be implemented by the caller of the library and
 *  that is passed to the AMQP connection. This interface contains all sorts
 *  of methods that are called when data needs to be sent, or when the 
 *  AMQP connection ends up in a broken state.
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
class ConnectionHandler
{
public:
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
    virtual void onData(Connection *connection, const char *buffer, size_t size) = 0;
    
    /**
     *  When the connection ends up in an error state this method is called.
     *  This happens when data comes in that does not match the AMQP protocol
     *  
     *  After this method is called, the connection no longer is in a valid
     *  state and can be used. In normal circumstances this method is not called.
     * 
     *  @todo   do we need this method, or only in the ChannelHandler class?
     *
     *  @param  connection      The connection that entered the error state
     *  @param  message         Error message
     */
    virtual void onConnectionError(Connection *connection, const std::string &message) = 0;
    
    /**
     *  Method that is called when the login attempt succeeded. After this method
     *  was called, the connection is ready to use
     *
     *  @param  connection      The connection that can now be used
     */
    virtual void onConnected(Connection *connection) = 0;
};

/**
 *  End of namespace
 */
}

