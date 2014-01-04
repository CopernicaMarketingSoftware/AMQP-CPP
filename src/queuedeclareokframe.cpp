/**
 *  QueueDeclareOKFrame.cpp
 */
 
#include "includes.h"
#include "queuedeclareokframe.h"

// setup namespace
namespace AMQP {
    
/**
 *  Process the frame
 *  @param  connection      The connection over which it was received
 *  @return bool            Was it succesfully processed?
 */
bool QueueDeclareOKFrame::process(ConnectionImpl *connection) 
{
    // check if we have a channel
    ChannelImpl *channel = connection->channel(this->channel());
    
    // what if channel doesn't exist?
    if (!channel) return false;
    
    // report to the handler
    channel->reportQueueDeclared(this->name(), this->messageCount(), this->consumerCount());
    
    // done
    return true;
}
    
}
