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
    
    // send a tune-ok frame back
    ConnectionTuneOKFrame okframe(channelMax(), frameMax(), heartbeat());
    
    // send it back
    connection->send(okframe);
    
    // and finally we start to open the frame
    ConnectionOpenFrame openframe(connection->vhost());
    
    // send the open frame
    connection->send(openframe);
    
    // done
    return true;
}

/**
 *  End of namespace
 */
}
