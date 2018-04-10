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
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <cstdint>
#include <stddef.h>

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Forward declarations
 */
class Connection;

/**
 *  Class definition
 */
class ConnectionHandler
{
public:
    /**
    *  Destructor
    */
    virtual ~ConnectionHandler() = default;

    /**
     *  Method that is called when the heartbeat frequency is negotiated
     *  between the server and the client. You normally do not have to
     *  override this method, because in the default implementation the
     *  suggested heartbeat is simply accepted by the client.
     *
     *  However, if you want to disable heartbeats, or when you want an
     *  alternative heartbeat interval, you can override this method
     *  to use an other interval. You should return 0 if you want to
     *  disable heartbeats.
     * 
     *  If heartbeats are enabled, you yourself are responsible to send
     *  out a heartbeat every *interval* number of seconds by calling
     *  the Connection::heartbeat() method.
     *
     *  @param  connection      The connection that suggested a heartbeat interval
     *  @param  interval        The suggested interval from the server
     *  @return uint16_t        The interval to use
     */
    virtual uint16_t onNegotiate(Connection *connection, uint16_t interval)
    {
        // default implementation, disable heartbeats
        return 0;
    }

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
     *  Method that is called when the server sends a heartbeat to the client
     *
     *  You do not have to do anything here, the client sends back a heartbeat
     *  frame automatically, but if you like, you can implement/override this
     *  method if you want to be notified of such heartbeats
     *
     *  @param  connection      The connection over which the heartbeat was received
     */
    virtual void onHeartbeat(Connection *connection) {}

    /**
     *  When the connection ends up in an error state this method is called.
     *  This happens when data comes in that does not match the AMQP protocol
     *
     *  After this method is called, the connection no longer is in a valid
     *  state and can no longer be used.
     *
     *  This method has an empty default implementation, although you are very
     *  much advised to implement it. When an error occurs, the connection
     *  is no longer usable, so you probably want to know.
     *
     *  @param  connection      The connection that entered the error state
     *  @param  message         Error message
     */
    virtual void onError(Connection *connection, const char *message) {}

    /**
     *  Method that is called when the login attempt succeeded. After this method
     *  is called, the connection is ready to use. This is the first method
     *  that is normally called after you've constructed the connection object.
     *
     *  According to the AMQP protocol, you must wait for the connection to become
     *  ready (and this onConnected method to be called) before you can start
     *  using the Connection object. However, this AMQP library will cache all
     *  methods that you call before the connection is ready, so in reality there
     *  is no real reason to wait for this method to be called before you send
     *  the first instructions.
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

