/**
 *  TcpChannel.h
 *
 *  Extended channel that can be constructed on top of a TCP connection
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 Copernica BV
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

