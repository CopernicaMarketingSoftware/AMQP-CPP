/**
 *  MethodFrame.cpp
 *
 *  @copyright 2014 Copernica BV
 */
#include "includes.h"
#include "exception.h"
#include "protocolexception.h"

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Process the frame
 *  @param  connection      The connection over which it was received
 *  @return bool            Was it succesfully processed?
 */
bool MethodFrame::process(ConnectionImpl *connection)
{
    // this is an exception
    throw ProtocolException("unimplemented frame type " + std::to_string(type()) + " class " + std::to_string(classID()) + " method " + std::to_string(methodID()));
    
    // unreachable
    return false;
}

/**
 *  End of namespace
 */
}

