/**
 *  ConnectionOpenOKFrame.cpp
 *
 *  @copyright 2014 Copernica BV
 */
#include "includes.h"
#include "connectionopenokframe.h"

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Process the frame
 *  @param  connection      The connection over which it was received
 *  @return bool            Was it succesfully processed?
 */
bool ConnectionOpenOKFrame::process(ConnectionImpl *connection)
{
    // all is ok, mark the connection as connected
    connection->setConnected();
    
    // done
    return true;
}
    
/**
 *  End of namespace
 */
}

