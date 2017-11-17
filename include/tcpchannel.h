/**
 *  TcpChannel.h
 *
 *  Extended channel that can be constructed on top of a TCP connection
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 - 2017 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class TcpChannel : public Channel
{
public:
    /**
     *  Constructor
     * 
     *  The passed in connection pointer must remain valid for the 
     *  lifetime of the channel.
     * 
     *  @param  connection
     */
    TcpChannel(TcpConnection *connection) :
        Channel(&connection->_connection) {}
        
    /**
     *  Destructor
     */
    virtual ~TcpChannel() {}
};

/**
 *  End of namespace
 */
}

