/**
 *  ExchangeDeclareOKFrame.cpp
 *
 *  @copyright 2014 Copernica BV
 */
 
#include "includes.h"
#include "exchangedeclareokframe.h"

// setup namespace
namespace AMQP {
    
/**
 *  Process the frame
 *  @param  connection      The connection over which it was received
 *  @return bool            Was it succesfully processed?
 */
bool ExchangeDeclareOKFrame::process(ConnectionImpl *connection)
{
    // we need the appropriate channel
    ChannelImpl *channel = connection->channel(this->channel());
    
    // channel does not exist
    if(!channel) return false;
    
    // report exchange declare ok
    channel->reportExchangeDeclared();
    
    // done
    return true;
}   

// end namespace
}
