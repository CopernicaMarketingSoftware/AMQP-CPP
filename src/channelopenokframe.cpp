/**
 *  ChannelOpenOkFrame.cpp
 *
 *  @copyright 2014 Copernica BV
 */
#include "includes.h"
#include "channelopenokframe.h"

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Process the frame
 *  @param  connection      The connection over which it was received
 *  @return bool            Was it succesfully processed?
 */
bool ChannelOpenOKFrame::process(ConnectionImpl *connection)
{
    // we need the appropriate channel
    ChannelImpl *channel = connection->channel(this->channel());
    
    // channel does not exist
    if(!channel) return false;    
    
    // report that the channel is open
    channel->reportReady();
    
    // done
    return true;
}

/**
 *  End of namespace
 */
}

