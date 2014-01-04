/**
 *  ConnectionTuneFrame.cpp
 *
 *  @copyright 2014
 */
#include "includes.h"
#include "connectiontuneframe.h"
#include "connectiontuneokframe.h"
#include "connectionopenframe.h"

/**
 *  Set up namespace
 */
namespace AMQP {
    
/**
 *  Process the frame
 *  @param  connection      The connection over which it was received
 *  @return bool            Was it succesfully processed?
 */
bool ConnectionTuneFrame::process(ConnectionImpl *connection)
{
    // @todo this is only allowed when the connection is set up
    
    
    // remember this in the connection
    connection->setCapacity(channelMax(), frameMax());
    
    // theoretically it is possible that the connection object gets destructed between sending the messages
    Monitor monitor(connection);
    
    // send it back
    connection->send(ConnectionTuneOKFrame(channelMax(), frameMax(), heartbeat()));
    
    // check if the connection object still exists
    if (!monitor.valid()) return true;
    
    // and finally we start to open the frame
    connection->send(ConnectionOpenFrame(connection->vhost()));
    
    // done
    return true;
}

/**
 *  End of namespace
 */
}
