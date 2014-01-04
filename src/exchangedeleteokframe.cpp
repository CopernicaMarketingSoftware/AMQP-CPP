/**
 *  ExchangeDeleteOKFrame.cpp
 * 
 *  @copyright 2014 Copernica BV
 */
#include "includes.h"
#include "exchangedeleteokframe.h"

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Process the frame
 *  @param  connection      The connection over which it was received
 *  @return bool            Was it succesfully processed?
 */
bool ExchangeDeleteOKFrame::process(ConnectionImpl *connection)
{
    // check if we have a channel
    ChannelImpl *channel = connection->channel(this->channel());
    
    // channel does not exist
    if(!channel) return false;
    
    // report to handler
    channel->reportExchangeDeleted();
    
    // done
    return true;
}

/**
 *  End of namespace
 */
}

