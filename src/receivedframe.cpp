/**
 *  ReceivedFrame.cpp
 *
 *  Implementation of the ReceivedFrame class
 *
 *  @copyright 2014 - 2017 Copernica BV
 */
#include "includes.h"
#include "heartbeatframe.h"
#include "confirmselectframe.h"
#include "confirmselectokframe.h"
#include "connectionstartokframe.h"
#include "connectionstartframe.h"
#include "connectionsecureframe.h"
#include "connectionsecureokframe.h"
#include "connectionopenokframe.h"
#include "connectionopenframe.h"
#include "connectiontuneokframe.h"
#include "connectiontuneframe.h"
#include "connectioncloseokframe.h"
#include "connectioncloseframe.h"
#include "channelopenframe.h"
#include "channelopenokframe.h"
#include "channelflowframe.h"
#include "channelflowokframe.h"
#include "channelcloseokframe.h"
#include "channelcloseframe.h"
#include "exchangedeclareframe.h"
#include "exchangedeclareokframe.h"
#include "exchangedeleteframe.h"
#include "exchangedeleteokframe.h"
#include "exchangebindframe.h"
#include "exchangebindokframe.h"
#include "exchangeunbindframe.h"
#include "exchangeunbindokframe.h"
#include "queuedeclareframe.h"
#include "queuedeclareokframe.h"
#include "queuebindframe.h"
#include "queuebindokframe.h"
#include "queuepurgeframe.h"
#include "queuepurgeokframe.h"
#include "queuedeleteframe.h"
#include "queuedeleteokframe.h"
#include "queueunbindframe.h"
#include "queueunbindokframe.h"
#include "basicqosframe.h"
#include "basicqosokframe.h"
#include "basicconsumeframe.h"
#include "basicconsumeokframe.h"
#include "basiccancelframe.h"
#include "basiccancelokframe.h"
#include "basicpublishframe.h"
#include "basicreturnframe.h"
#include "basicdeliverframe.h"
#include "basicgetframe.h"
#include "basicgetokframe.h"
#include "basicgetemptyframe.h"
#include "basicackframe.h"
#include "basicnackframe.h"
#include "basicrejectframe.h"
#include "basicrecoverasyncframe.h"
#include "basicrecoverframe.h"
#include "basicrecoverokframe.h"
#include "transactionselectframe.h"
#include "transactionselectokframe.h"
#include "transactioncommitframe.h"
#include "transactioncommitokframe.h"
#include "transactionrollbackframe.h"
#include "transactionrollbackokframe.h"
#include "bodyframe.h"
#include "basicheaderframe.h"

#define TYPE_INVALID 0
#define END_OF_FRAME 206

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Constructor
 *  @param  buffer      Binary buffer
 *  @param  max         Max size for a frame
 */
ReceivedFrame::ReceivedFrame(const Buffer &buffer, uint32_t max) : InBuffer(buffer)
{
    // we need enough room for type, channel, the payload size, 
    // the the end-of-frame byte is not yet necessary
    if (buffer.size() < 7) return;
    
    // get the information
    _type = nextUint8();
    _channel = nextUint16();
    _payloadSize = nextUint32();

    // is the frame size bigger than the max frame size?
    if (max > 0 && _payloadSize > max - 8) throw ProtocolException("frame size exceeded");

    // check if the buffer is big enough to contain all data
    if (!complete()) return;

    // buffer is big enough, check for a valid end-of-frame marker
    if ((uint8_t)buffer.byte(_payloadSize+7) == END_OF_FRAME) return;

    // the frame is invalid because it does not end with the expected char
    throw ProtocolException("invalid end of frame marker");
}

/**
 *  Have we received the header of the frame
 *  @return bool
 */
bool ReceivedFrame::header() const
{
    return _buffer.size() >= 7;
}

/**
 *  Is this a complete frame?
 *  @return integer
 */
bool ReceivedFrame::complete() const
{
    return _buffer.size() >= _payloadSize + 8;
}

/**
 *  Process the received frame
 *  @param  connection
 *  @return bool
 */
bool ReceivedFrame::process(ConnectionImpl *connection)
{
    // check the type
    switch (_type)
    {
        case 1:     return processMethodFrame(connection);
        case 2:     return processHeaderFrame(connection);
        case 3:     return BodyFrame(*this).process(connection);
        case 4:     return HeartbeatFrame(*this).process(connection);
        case 8:     return HeartbeatFrame(*this).process(connection);
    }
    
    // this is a problem
    throw ProtocolException("unrecognized frame type " + std::to_string(_type));
}

/**
 *  Process a method frame
 *  @param  connection
 *  @return bool
 */
bool ReceivedFrame::processMethodFrame(ConnectionImpl *connection)
{
    // read the class id from the method
    uint16_t classID = nextUint16();
    
    // construct frame based on method id
    switch (classID)
    {
        case 10:    return processConnectionFrame(connection);
        case 20:    return processChannelFrame(connection);
        case 40:    return processExchangeFrame(connection);
        case 50:    return processQueueFrame(connection);
        case 60:    return processBasicFrame(connection);
        case 85:    return processConfirmFrame(connection);
        case 90:    return processTransactionFrame(connection);
    }

    // this is a problem
    throw ProtocolException("unrecognized method frame class " + std::to_string(classID));
}

/**
 *  Process a connection frame
 *  @param  connection
 *  @return bool
 */
bool ReceivedFrame::processConnectionFrame(ConnectionImpl *connection)
{
    // read the method id from the method
    uint16_t methodID = nextUint16();

    // construct frame based on method id
    switch (methodID)
    {
        case 10:    return ConnectionStartFrame(*this).process(connection);
        case 11:    return ConnectionStartOKFrame(*this).process(connection);
        case 20:    return ConnectionSecureFrame(*this).process(connection);
        case 21:    return ConnectionSecureOKFrame(*this).process(connection);
        case 30:    return ConnectionTuneFrame(*this).process(connection);
        case 31:    return ConnectionTuneOKFrame(*this).process(connection);
        case 40:    return ConnectionOpenFrame(*this).process(connection);
        case 41:    return ConnectionOpenOKFrame(*this).process(connection);
        case 50:    return ConnectionCloseFrame(*this).process(connection);
        case 51:    return ConnectionCloseOKFrame(*this).process(connection);
    }

    // this is a problem
    throw ProtocolException("unrecognized connection frame method " + std::to_string(methodID));
}

/**
 *  Process a channel frame
 *  @param  connection
 *  @return bool
 */
bool ReceivedFrame::processChannelFrame(ConnectionImpl *connection)
{
    // read the method id from the method
    uint16_t methodID = nextUint16();
    
    // construct frame based on method id
    switch (methodID)
    {
        case 10:    return ChannelOpenFrame(*this).process(connection);
        case 11:    return ChannelOpenOKFrame(*this).process(connection);
        case 20:    return ChannelFlowFrame(*this).process(connection);
        case 21:    return ChannelFlowOKFrame(*this).process(connection);
        case 40:    return ChannelCloseFrame(*this).process(connection);
        case 41:    return ChannelCloseOKFrame(*this).process(connection);
    }

    // this is a problem
    throw ProtocolException("unrecognized channel frame method " + std::to_string(methodID));
}

/**
 *  Process an exchange frame
 *  @param  connection
 *  @return bool
 */
bool ReceivedFrame::processExchangeFrame(ConnectionImpl *connection)
{
    // read the method id from the method
    uint16_t methodID = nextUint16();

    // construct frame based on method id
    switch(methodID)
    {
        case 10:    return ExchangeDeclareFrame(*this).process(connection);
        case 11:    return ExchangeDeclareOKFrame(*this).process(connection);
        case 20:    return ExchangeDeleteFrame(*this).process(connection);
        case 21:    return ExchangeDeleteOKFrame(*this).process(connection);
        case 30:    return ExchangeBindFrame(*this).process(connection);
        case 31:    return ExchangeBindOKFrame(*this).process(connection);
        case 40:    return ExchangeUnbindFrame(*this).process(connection);
                    // contrary to the rule of good continuation, exchangeunbindok
                    // has method ID 51, instead of (the expected) 41. This is tested
                    // and it really has ID 51.  
        case 51:    return ExchangeUnbindOKFrame(*this).process(connection);
    }

    // this is a problem
    throw ProtocolException("unrecognized exchange frame method " + std::to_string(methodID));
}

/**
 *  Process a queue frame
 *  @param  connection
 *  @return bool
 */
bool ReceivedFrame::processQueueFrame(ConnectionImpl *connection)
{
    // read the method id from the method
    uint16_t methodID = nextUint16();

    // construct frame based on method id
    switch (methodID)
    {
        case 10:    return QueueDeclareFrame(*this).process(connection);
        case 11:    return QueueDeclareOKFrame(*this).process(connection);
        case 20:    return QueueBindFrame(*this).process(connection);
        case 21:    return QueueBindOKFrame(*this).process(connection);
        case 30:    return QueuePurgeFrame(*this).process(connection);
        case 31:    return QueuePurgeOKFrame(*this).process(connection);
        case 40:    return QueueDeleteFrame(*this).process(connection);
        case 41:    return QueueDeleteOKFrame(*this).process(connection);
        case 50:    return QueueUnbindFrame(*this).process(connection);
        case 51:    return QueueUnbindOKFrame(*this).process(connection);
    }

    // this is a problem
    throw ProtocolException("unrecognized queue frame method " + std::to_string(methodID));
}

/**
 *  Process a basic frame
 *  @param  connection
 *  @return bool
 */
bool ReceivedFrame::processBasicFrame(ConnectionImpl *connection)
{
    // read the method id from the method
    uint16_t methodID = nextUint16();

    // construct frame based on method id
    switch (methodID)
    {
        case 10:    return BasicQosFrame(*this).process(connection);
        case 11:    return BasicQosOKFrame(*this).process(connection);
        case 20:    return BasicConsumeFrame(*this).process(connection);
        case 21:    return BasicConsumeOKFrame(*this).process(connection);
        case 30:    return BasicCancelFrame(*this).process(connection);
        case 31:    return BasicCancelOKFrame(*this).process(connection);
        case 40:    return BasicPublishFrame(*this).process(connection);
        case 50:    return BasicReturnFrame(*this).process(connection);
        case 60:    return BasicDeliverFrame(*this).process(connection);
        case 70:    return BasicGetFrame(*this).process(connection);
        case 71:    return BasicGetOKFrame(*this).process(connection);
        case 72:    return BasicGetEmptyFrame(*this).process(connection);
        case 80:    return BasicAckFrame(*this).process(connection);
        case 90:    return BasicRejectFrame(*this).process(connection);
        case 100:   return BasicRecoverAsyncFrame(*this).process(connection);
        case 110:   return BasicRecoverFrame(*this).process(connection);
        case 111:   return BasicRecoverOKFrame(*this).process(connection);
        case 120:   return BasicNackFrame(*this).process(connection);
    }

    // this is a problem
    throw ProtocolException("unrecognized basic frame method " + std::to_string(methodID));
}

/**
 *  Process a confirm frame
 *  @param  connection
 *  @return bool
 */
bool ReceivedFrame::processConfirmFrame(ConnectionImpl *connection)
{
    // read the method id
    uint16_t methodID = nextUint16();

    // construct frame based on method id
    switch (methodID)
    {
        case 10:    return ConfirmSelectFrame(*this).process(connection);
        case 11:    return ConfirmSelectOKFrame(*this).process(connection);
    }

    // this is a problem
    throw ProtocolException("unrecognized confirm frame method " + std::to_string(methodID));
}

/**
 *  Process a transaction frame
 *  @param  connection
 *  @return bool
 */
bool ReceivedFrame::processTransactionFrame(ConnectionImpl *connection)
{
    // read the method id
    uint16_t methodID = nextUint16();

    // construct frame based on method id
    switch (methodID)
    {
        case 10:    return TransactionSelectFrame(*this).process(connection);
        case 11:    return TransactionSelectOKFrame(*this).process(connection);
        case 20:    return TransactionCommitFrame(*this).process(connection);
        case 21:    return TransactionCommitOKFrame(*this).process(connection);
        case 30:    return TransactionRollbackFrame(*this).process(connection);
        case 31:    return TransactionRollbackOKFrame(*this).process(connection);
    }

    // this is a problem
    throw ProtocolException("unrecognized transaction frame method " + std::to_string(methodID));
}

/**
 *  Process a header frame
 *  @param  connection
 *  @return bool
 */
bool ReceivedFrame::processHeaderFrame(ConnectionImpl *connection)
{
    // read the class id from the method
    uint16_t classID = nextUint16();
    
    // construct a frame based on class id
    switch (classID)
    {
        case 60:    return BasicHeaderFrame(*this).process(connection);
    }

    // this is a problem
    throw ProtocolException("unrecognized header frame class " + std::to_string(classID));
}

/**
 *  End of namespace
 */
}

