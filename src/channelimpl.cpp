/**
 *  Channel.cpp
 *
 *  Implementation for a channel
 *
 *  @copyright 2014 Copernica BV
 */
#include "includes.h"
#include "basicdeliverframe.h"
#include "messageimpl.h"
#include "channelopenframe.h"
#include "channelflowframe.h"
#include "channelcloseokframe.h"
#include "channelcloseframe.h"
#include "transactionselectframe.h"
#include "transactioncommitframe.h"
#include "transactionrollbackframe.h"
#include "exchangedeclareframe.h"
#include "exchangedeleteframe.h"
#include "exchangebindframe.h"
#include "exchangeunbindframe.h"
#include "queuedeclareframe.h"
#include "queuebindframe.h"
#include "queueunbindframe.h"
#include "queuepurgeframe.h"
#include "queuedeleteframe.h"
#include "basicpublishframe.h"
#include "basicheaderframe.h"
#include "bodyframe.h"
#include "basicqosframe.h"
#include "basicconsumeframe.h"
#include "basiccancelframe.h"
#include "basicackframe.h"

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Construct a channel object
 *  @param  parent
 *  @param  connection
 *  @param  handler
 */
ChannelImpl::ChannelImpl(Channel *parent, Connection *connection, ChannelHandler *handler) :
    _parent(parent),
    _connection(connection),
    _handler(handler)
{
    // add the channel to the connection
    _id = connection->_implementation.add(this);
    
    // check if the id is valid
    if (_id == 0)
    {
        // this is invalid
        _state = state_closed;
        
        // invalid id, this channel can not exist
        handler->onError(_parent, "Max number of channels reached");
    }
    else
    {
        // busy connecting
        _state = state_connected;
        
        // valid id, send a channel open frame
        send(ChannelOpenFrame(_id));
    }
}

/**
 *  Destructor
 */
ChannelImpl::~ChannelImpl()
{
    // remove incoming message 
    if (_message) delete _message;
    _message = nullptr;
    
    // remove this channel from the connection
    _connection->_implementation.remove(this);
    
    // leap out if already disconnected
    if (!connected()) return;
    
    // close the channel now
    // @todo is this ok?
    close();
    
    // do we have 
}

/**
 *  Pause deliveries on a channel
 * 
 *  This will stop all incoming messages
 *  
 *  This method returns true if the request to pause has been sent to the
 *  broker. This does not necessarily mean that the channel is already
 *  paused. 
 * 
 *  @return bool
 */
bool ChannelImpl::pause()
{
    // must be connected
    if (!connected()) return false;
    
    // send a flow frame
    send(ChannelFlowFrame(_id, false));
    
    // done
    return true;
}

/**
 *  Resume a paused channel
 * 
 *  @return bool
 */
bool ChannelImpl::resume()
{
    // must be connected
    if (!connected()) return false;
    
    // send a flow frame
    send(ChannelFlowFrame(_id, true));

    // done
    return true;
}

/**
 *  Start a transaction
 *  @return bool
 */
bool ChannelImpl::startTransaction()
{
    // must be connected
    if (!connected()) return false;
    
    // send a flow frame
    send(TransactionSelectFrame(_id));

    // done
    return true;
}    

/**
 *  Commit the current transaction
 *  @return bool
 */
bool ChannelImpl::commitTransaction()
{
    // must be connected
    if (!connected()) return false;
    
    // send a flow frame
    send(TransactionCommitFrame(_id));

    // done
    return true;
}

/**
 *  Rollback the current transaction
 *  @return bool
 */
bool ChannelImpl::rollbackTransaction()
{
    // must be connected
    if (!connected()) return false;
    
    // send a flow frame
    send(TransactionRollbackFrame(_id));

    // done
    return true;
}

/**
 *  Close the current channel
 *  @return bool
 */
bool ChannelImpl::close()
{
    // must be connected
    if (!connected()) return false;
    
    // send a flow frame
    send(ChannelCloseFrame(_id));

    // now it is closed
    _state = state_closed;

    // done
    return true;
}

/**
 *  declare an exchange
 *  @param  name        name of the exchange to declare
 *  @param  type        type of exchange
 *  @param  flags       additional settings for the exchange
 *  @param  arguments   additional arguments
 *  @return bool
 */
bool ChannelImpl::declareExchange(const std::string &name, ExchangeType type, int flags, const Table &arguments)
{
    // must be connected
    if(!connected()) return false;
    
    std::string exchangeType;
    if(type == ExchangeType::fanout) exchangeType = "fanout";
    if(type == ExchangeType::direct) exchangeType = "direct";
    if(type == ExchangeType::topic)  exchangeType = "topic";
    if(type == ExchangeType::headers)exchangeType = "headers";
    // send declare exchange frame
    send(ExchangeDeclareFrame(_id, name, exchangeType, (flags & passive) != 0, (flags & durable) != 0, (flags & nowait) != 0, arguments));
    
    // done
    return true;
}

/**
 *  bind an exchange
 *  @param  source      exchange which binds to target
 *  @param  target      exchange to bind to
 *  @param  routingKey  routing key
 *  @param  flags       additional flags
 *  @param  arguments   additional arguments for binding
 *  @return bool
 */
bool ChannelImpl::bindExchange(const std::string &source, const std::string &target, const std::string &routingkey, int flags, const Table &arguments)
{
    // must be connected
    if(!connected()) return false;
    
    // send exchange bind frame
    send(ExchangeBindFrame(_id, target, source, routingkey, (flags & nowait) != 0, arguments));
    
    //done
    return true;
}

/**
 *  unbind two exchanges
 *  @param  source      the source exchange
 *  @param  target      the target exchange
 *  @param  routingkey  the routing key
 *  @param  flags       optional flags
 *  @param  arguments   additional unbind arguments
 *  @return bool
 */
bool ChannelImpl::unbindExchange(const std::string &source, const std::string &target, const std::string &routingkey, int flags, const Table &arguments)
{
    // must be connected
    if (!connected()) return false;
    
    // send exchange unbind frame
    send(ExchangeUnbindFrame(_id, target, source, routingkey, (flags & nowait) != 0, arguments));
    
    // done
    return true;
}

/**
 *  remove an exchange
 *  @param  name        name of the exchange to remove
 *  @param  flags       additional settings for deleting the exchange
 *  @return bool
 */
bool ChannelImpl::removeExchange(const std::string &name, int flags)
{
    // must be connected
    if (!connected()) return false;
    
    // send delete exchange frame
    send(ExchangeDeleteFrame(_id, name, (flags & ifunused) != 0, (flags & nowait) != 0));
    
    // done
    return true;
}

/**
 *  declare a queue
 *  @param  name        queue name
 *  @param  flags       additional settings for the queue
 *  @param  arguments   additional arguments
 *  @return bool
 */
bool ChannelImpl::declareQueue(const std::string &name, int flags, const Table &arguments)
{
    // must be connected
    if (!connected()) return false;
    
    // send the queuedeclareframe
    send(QueueDeclareFrame(_id, name, (flags & passive) != 0, (flags & durable) != 0, (flags & durable) != 0, (flags & autodelete) != 0, (flags & nowait) != 0, arguments));
    
    // done
    return true;
}

/**
 *  Bind a queue to an exchange
 *  @param  exchangeName    name of the exchange to bind to
 *  @param  queueName       name of the queue
 *  @param  routingkey      routingkey
 *  @param  flags           additional flags
 *  @param  arguments       additional arguments
 *  @return bool
 */
bool ChannelImpl::bindQueue(const std::string &exchangeName, const std::string &queueName, const std::string &routingkey, int flags, const Table &arguments)
{
    // must be connected
    if(!connected()) return false;
    
    // send the bind queue frame
    send(QueueBindFrame(_id, queueName, exchangeName, routingkey, (flags & nowait) != 0, arguments));
    
    // done
    return true;
}

/**
 *  Unbind a queue from an exchange
 *  @param  exchange    the source exchange
 *  @param  queue       the target queue
 *  @param  routingkey  the routing key
 *  @param  arguments   additional bind arguments
 *  @return bool
 */
bool ChannelImpl::unbindQueue(const std::string &exchange, const std::string &queue, const std::string &routingkey, const Table &arguments)
{
    // must be connected
    if(!connected()) return false;
    
    // send the unbind queue frame
    send(QueueUnbindFrame(_id, queue, exchange, routingkey, arguments));
    
    // done
    return true;
}

/**
 *  Purge a queue
 *  @param  queue       queue to purge
 *  @param  flags       additional flags
 *  @return bool    
 */
bool ChannelImpl::purgeQueue(const std::string &name, int flags)
{
    // must be connected
    if(!connected()) return false;
    
    // send the queue purge frame
    send(QueuePurgeFrame(_id, name, (flags & nowait) != 0));
    
    // done
    return true;
}

/**
 *  Remove a queue
 *  @param  queue       queue to remove
 *  @param  flags       additional flags
 *  @return bool
 */
bool ChannelImpl::removeQueue(const std::string &name, int flags)
{
    // must be connected
    if(!connected()) return false;
    
    // send the remove queue frame
    send(QueueDeleteFrame(_id, name, (flags & ifunused) != 0,(flags & ifempty) != 0,(flags & nowait) != 0));
    
    // done
    return true;
}

/**
 *  Publish a message to an exchange
 * 
 *  The following flags can be used
 * 
 *      -   mandatory   if set, an unroutable message will be reported to the channel handler with the onReturned method
 *      -   immediate   if set, a message that could not immediately be consumed is returned to the onReturned method
 * 
 *  @todo   implement to onReturned() method
 * 
 *  @param  exchange    the exchange to publish to
 *  @param  routingkey  the routing key
 *  @param  flags       optional flags (see above)
 *  @param  envelope    the full envelope to send
 *  @param  message     the message to send
 *  @param  size        size of the message
 */
bool ChannelImpl::publish(const std::string &exchange, const std::string &routingKey, int flags, const Envelope &envelope)
{
    // @todo prevent crash when connection is destructed
    
    // @todo do not copy the entire buffer to individual frames
    
    // send the publish frame
    send(BasicPublishFrame(_id, exchange, routingKey, flags & mandatory, flags & immediate));

    // send header
    send(BasicHeaderFrame(_id, envelope));
    
    // the max payload size is the max frame size minus the bytes for headers and trailer
    uint32_t maxpayload = _connection->_implementation.maxPayload();
    uint32_t bytessent = 0;
    
    // the buffer
    const char *data = envelope.body();
    uint32_t bytesleft = envelope.bodySize();
    
    // split up the body in multiple frames depending on the max frame size
    while (bytesleft > 0)
    {
        // size of this chunk
        uint32_t chunksize = std::min(maxpayload, bytesleft);
        
        // send out a body frame
        send(BodyFrame(_id, data + bytessent, chunksize));
        
        // update counters
        bytessent += chunksize;
        bytesleft -= chunksize;
    }
    
    // done
    return true;
}

/**
 *  Set the Quality of Service (QOS) for this channel
 *  @param  prefetchCount       maximum number of messages to prefetch
 *  @return bool                whether the Qos frame is sent.
 */
bool ChannelImpl::setQos(uint16_t prefetchCount)
{
    // send a qos frame
    send(BasicQosFrame(_id, prefetchCount, false));
    
    // done
    return true;
}

/**
 *  Tell the RabbitMQ server that we're ready to consume messages
 *  @param  queue               the queue from which you want to consume
 *  @param  tag                 a consumer tag that will be associated with this consume operation
 *  @param  flags               additional flags
 *  @param  arguments           additional arguments
 *  @return bool
 */
bool ChannelImpl::consume(const std::string &queue, const std::string &tag, int flags, const Table &arguments)
{
    // send a consume frame
    send(BasicConsumeFrame(_id, queue, tag, flags & nolocal, flags & noack, flags & exclusive, flags & nowait, arguments));
    
    // done
    return true;
}

/**
 *  Cancel a running consumer
 *  @param  tag                 the consumer tag
 *  @param  flags               optional flags
 */
bool ChannelImpl::cancel(const std::string &tag, int flags)
{
    // send a cancel frame
    send(BasicCancelFrame(_id, tag, flags & nowait));
    
    // done
    return true;
}

/**
 *  Acknoledge a message
 *  @param  deliveryTag         the delivery tag
 *  @param  flags               optional flags
 *  @return bool
 */
bool ChannelImpl::ack(uint64_t deliveryTag, int flags)
{
    // send an ack frame
    send(BasicAckFrame(_id, deliveryTag, flags & multiple));
    
    // done
    return true;
}

/**
 *  Send a frame over the channel
 *  @param  frame       frame to send
 *  @return size_t      number of bytes sent
 */
size_t ChannelImpl::send(const Frame &frame)
{
    // send to tcp connection
    return _connection->_implementation.send(frame);
}

/**
 *  Report the received message
 */
void ChannelImpl::reportReceived()
{
    if (_handler) _handler->onReceived(_parent, *_message);
}

/**
 *  Create an incoming message
 *  @param  frame
 *  @return MessageImpl
 */
MessageImpl *ChannelImpl::message(const BasicDeliverFrame &frame)
{
    // it should not be possible that a message already exists, but lets check it anyhow
    if (_message) delete _message;
    
    // construct a message
    return _message = new MessageImpl(frame);
}

/**
 *  End of namespace
 */
}

