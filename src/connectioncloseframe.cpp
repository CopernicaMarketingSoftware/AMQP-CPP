/**
 *  ConnectionCloseFrame.cpp
 * 
 *  @copyright 2014 Copernica BV
 */
#include "includes.h"
#include "connectioncloseframe.h"
#include "connectioncloseokframe.h"

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Process the frame
 *  @param  connection      The connection over which it was received
 *  @return bool            Was it succesfully processed?
 */
bool ConnectionCloseFrame::process(ConnectionImpl *connection)
{
    // @todo    connection could be destructed after frame was sent
    
    // send back the ok frame
    connection->send(ConnectionCloseOKFrame());
    
    // no need to check for a channel, the error is connection wide
    // report the error on the connection
    connection->reportError(text());
    
    // done
    return true;
}

/**
 *  End of namespace
 */
}

