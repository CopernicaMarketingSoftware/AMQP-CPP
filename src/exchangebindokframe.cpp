/**
 *  Exchangebindokframe.cpp
 */
 
#include "includes.h"
#include "exchangebindokframe.h"

// setup namespace
namespace AMQP {

/**
 *  Process the frame
 *  @param  connection      The connection over which it was received
 *  @return bool            Was it succesfully processed?
 */
bool ExchangeBindOKFrame::process(ConnectionImpl *connection)
{
    // check if we have a channel
    ChannelImpl *channel = connection->channel(this->channel());
    
    // channel does not exist
    if(!channel) return false;

    // report to handler
    channel->reportExchangeBound();
    
    // done
    return true;
}

// end namespace
}
