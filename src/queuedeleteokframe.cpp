/**
 *  QueueDeleteOKFrame.cpp
 */
 
#include "includes.h"
#include "queuedeleteokframe.h"

// setup namespace
namespace AMQP {

/**
 *  Process the frame
 *  @param  connection      The connection over which it was received
 *  @return bool            Was it succesfully processed?
 */
bool QueueDeleteOKFrame::process(ConnectionImpl *connection)
{
    // check if we have a channel
    ChannelImpl *channel = connection->channel(this->channel());
    
    // channel does not exist
    if(!channel) return false;
    
    // report queue deletion success
    channel->reportQueueDeleted(this->messageCount());
    
    // done
    return true;
}


// end namespace
}
