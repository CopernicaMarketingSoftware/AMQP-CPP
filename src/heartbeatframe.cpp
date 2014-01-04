/**
 *  HeartbeatFrame.cpp
 *
 *  @copyright 2014 Copernica BV
 */
#include "includes.h"
#include "heartbeatframe.h"

/**
 *  Namespace
 */
namespace AMQP {

/**
 *  Process the frame
 *  @param  connection      The connection over which it was received
 *  @return bool            Was it succesfully processed?
 */
bool HeartbeatFrame::process(ConnectionImpl *connection)
{
    // send back the same frame
    connection->send(*this);

    // done
    return true;
}

/**
 *  End namespace
 */
}

