/**
 *  ChannelFlowOkFrame.cpp
 *
 *  @copyright 2014 Copernica BV
 */
#include "includes.h"
#include "channelflowokframe.h"

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Process the frame
 *  @param  connection      The connection over which it was received
 *  @return bool            Was it succesfully processed?
 */
bool ChannelFlowOKFrame::process(ConnectionImpl *connection)
{
    // we need the appropriate channel
    ChannelImpl *channel = connection->channel(this->channel());
    
    // channel does not exist
    if(!channel) return false;    
    
    // is the flow active?
    if (active()) channel->reportResumed();
    else channel->reportPaused();
    
    // done
    return true;
}

/**
 *  End of namespace
 */
}

