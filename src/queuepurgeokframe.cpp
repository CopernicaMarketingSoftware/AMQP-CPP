/**
 *  QueuePurgeOKFrame.cpp
 */

#include "includes.h"
#include "queuepurgeokframe.h"

// setup namespace
namespace AMQP {

/**
 *  Process the frame
 *  @param  connection      The connection over which it was received
 *  @return bool            Was it succesfully processed?
 */
bool QueuePurgeOKFrame::process(ConnectionImpl *connection)
{
    // check if we have a channel
    ChannelImpl *channel = connection->channel(this->channel());
    
    // channel does not exist
    if(!channel) return false;

    // report queue purge success
    channel->reportQueuePurged(this->messageCount());
    
    // done
    return true;
}

// end namespace
}
