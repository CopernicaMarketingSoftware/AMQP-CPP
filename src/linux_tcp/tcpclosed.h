/**
 *  TcpClosed.h
 *
 *  Class that is used when the TCP connection ends up in a closed state
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 - 2016 Copernica BV
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
class TcpClosed : public TcpState
{
public:
    /**
     *  Constructor
     *  @param  connection  The parent TcpConnection object
     *  @param  handler     User supplied handler
     */
    TcpClosed(TcpConnection *connection, TcpHandler *handler) : 
        TcpState(connection, handler) {}

    /**
     *  Constructor
     *  @param  state       The to-be-copied state
     */
    TcpClosed(const TcpState *state) : 
        TcpState(state) {}
    
    /**
     *  Destructor
     */
    virtual ~TcpClosed() noexcept = default;
};
    
/**
 *  End of namespace
 */
}

