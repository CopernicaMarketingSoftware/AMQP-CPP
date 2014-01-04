/**
 *  ChannelCloseOkFrame.cpp
 *
 *  @copyright 2014 Copernica BV
 */
#include "includes.h"
#include "channelcloseokframe.h"

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Process the frame
 *  @param  connection      The connection over which it was received
 *  @return bool            Was it succesfully processed?
 */
bool ChannelCloseOKFrame::process(ConnectionImpl *connection)
{
    // we need the appropriate channel
    ChannelImpl *channel = connection->channel(this->channel());
    
    // channel does not exist
    if(!channel) return false;    
    
    // report that the channel is closed
    channel->reportClosed();
    
    // done
    return true;
}

/**
 *  End of namespace
 */
}

