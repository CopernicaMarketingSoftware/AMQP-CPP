/**
 *  ChannelCloseFrame.cpp
 *
 *  @copyright 2014 Copernica BV
 */
#include "includes.h"
#include "channelcloseframe.h"
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
bool ChannelCloseFrame::process(ConnectionImpl *connection)
{
    // check if we have a channel
    ChannelImpl *channel = connection->channel(this->channel());
    
    // send back an ok frame
    connection->send(ChannelCloseOKFrame(this->channel()));
    
    // what if channel doesn't exist?
    if (!channel) return false;
    
    // report to the handler
    channel->reportChannelError(text());
    
    // done
    return true;
}

/**
 *  End of namespace
 */
}

