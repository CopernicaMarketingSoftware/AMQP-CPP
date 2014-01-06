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
     *  state and can no longer be used.
     * 
     *  This method has an empty default implementation, although you are very
     *  much advised to implement it. Because when an error occurs, the connection
     *  is no longer usable, so you probably want to know.
     * 
     *  @param  connection      The connection that entered the error state
     *  @param  message         Error message
     */
    virtual void onError(Connection *connection, const std::string &message) {}
    
    /**
     *  Method that is called when the login attempt succeeded. After this method
     *  was called, the connection is ready to use. This is the first method
     *  that is normally called after you've constructed the connection object.
     * 
     *  According to the AMQP protocol, you must wait for the connection to become
     *  ready (and this onConnected method to be called) before you can start
     *  using the Connection object. However, this AMQP library will cache all
     *  methods that you call before the connection is ready, so in reality there
     *  is no real reason to wait.
     *
     *  @param  connection      The connection that can now be used
     */
    virtual void onConnected(Connection *connection) {}
    
    /**
     *  Method that is called when the connection was closed.
     * 
     *  This is the counter part of a call to Connection::close() and it confirms
     *  that the connection was correctly closed.
     * 
     *  @param  connection      The connection that was closed and that is now unusable
     */
    virtual void onClosed(Connection *connection) {}
    
};

/**
 *  End of namespace
 */
}

