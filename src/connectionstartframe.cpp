/**
 *  ConnectionStartFrame.h
 *
 *  @copyright 2014 Copernica BV
 */
#include "includes.h"
#include "connectionstartframe.h"
#include "connectionstartokframe.h"

/**
 *  Set up namespace
 */
namespace AMQP {
    
/**
 *  Process the connection start frame
 *  @param  connection
 *  @return bool
 *  @internal
 */
bool ConnectionStartFrame::process(ConnectionImpl *connection)
{
    // @todo we must still be in protocol handshake mode
    
    
    // the peer properties
    Table properties;
    
    // fill the peer properties
    properties["product"] = "Copernica AMQP library";
    properties["version"] = "0.1";
    properties["platform"] = "Ubuntu";
    properties["copyright"] = "Copyright 2014 Copernica BV";
    properties["information"] = "";
    
    // move connection to handshake mode
    connection->setProtocolOk();
    
    // send back a connection start ok frame
    connection->send(ConnectionStartOKFrame(properties, "PLAIN", connection->login().saslPlain(), "en_US"));
    
    // done
    return true;
}
    
/**
 *  End of namespace
 */
}

