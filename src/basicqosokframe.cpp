/**
 *  BasicQosOkFrame.cpp
 *
 *  @copyright 2014 Copernica BV
 */
#include "includes.h"
#include "basicqosokframe.h"

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Process the frame
 *  @param  connection      The connection over which it was received
 *  @return bool            Was it succesfully processed?
 */
bool BasicQosOKFrame::process(ConnectionImpl *connection)
{
    // we need the appropriate channel
    ChannelImpl *channel = connection->channel(this->channel());
    
    // channel does not exist
    if (!channel) return false;    
    
    // report
    channel->reportQosSet();
    
    // done
    return true;
    

}

/**
 *  End of namespace
 */
}

