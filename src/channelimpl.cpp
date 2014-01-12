/**
 *  Channel.cpp
 *
 *  Implementation for a channel
 *
 *  @copyright 2014 Copernica BV
 */
#include "includes.h"
#include "basicdeliverframe.h"
#include "basicreturnframe.h"
#include "messageimpl.h"
#include "consumedmessage.h"
#include "returnedmessage.h"
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
#include "basicnackframe.h"
#include "basicrecoverframe.h"

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
    _connection(&connection->_implementation),
    _handler(handler)
{
    // add the channel to the connection
    _id = _connection->add(this);
    
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
    
    // remove this channel from the connection (but not if the connection is already destructed)
    if (_connection) _connection->remove(this);
    
    // close the channel now
    close();
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
    // send a flow frame
    return send(ChannelFlowFrame(_id, false));
}

/**
 *  Resume a paused channel
 * 
 *  @return bool
 */
bool ChannelImpl::resume()
{
    // send a flow frame
    return send(ChannelFlowFrame(_id, true));
}

/**
 *  Start a transaction
 *  @return bool
 */
bool ChannelImpl::startTransaction()
{
    // send a flow frame
    return send(TransactionSelectFrame(_id));
}    

/**
 *  Commit the current transaction
 *  @return bool
 */
bool ChannelImpl::commitTransaction()
{
    // send a flow frame
    return send(TransactionCommitFrame(_id));
}

/**
 *  Rollback the current transaction
 *  @return bool
 */
bool ChannelImpl::rollbackTransaction()
{
    // send a flow frame
    return send(TransactionRollbackFrame(_id));
}

/**
 *  Close the current channel
 *  @return bool
 */
bool ChannelImpl::close()
{
    // channel could be dead after send operation, we need to monitor that
    Monitor monitor(this);

    // send a flow frame
    if (!send(ChannelCloseFrame(_id))) return false;

    // leap out if channel was destructed
    if (!monitor.valid()) return true;

    // now it is closing
    _state = state_closing;

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
    // convert exchange type
    std::string exchangeType;
    if (type == ExchangeType::fanout) exchangeType = "fanout";
    if (type == ExchangeType::direct) exchangeType = "direct";
    if (type == ExchangeType::topic)  exchangeType = "topic";
    if (type == ExchangeType::headers)exchangeType = "headers";

    // send declare exchange frame
    return send(ExchangeDeclareFrame(_id, name, exchangeType, flags & passive, flags & durable, flags & nowait, arguments));
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
    // send exchange bind frame
    return send(ExchangeBindFrame(_id, target, source, routingkey, flags & nowait, arguments));
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
    // send exchange unbind frame
    return send(ExchangeUnbindFrame(_id, target, source, routingkey, flags & nowait, arguments));
}

/**
 *  remove an exchange
 *  @param  name        name of the exchange to remove
 *  @param  flags       additional settings for deleting the exchange
 *  @return bool
 */
bool ChannelImpl::removeExchange(const std::string &name, int flags)
{
    // send delete exchange frame
    return send(ExchangeDeleteFrame(_id, name, flags & ifunused, flags & nowait));
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
    // send the queuedeclareframe
    return send(QueueDeclareFrame(_id, name, flags & passive, flags & durable, flags & exclusive, flags & autodelete, flags & nowait, arguments));
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
    // send the bind queue frame
    return send(QueueBindFrame(_id, queueName, exchangeName, routingkey, flags & nowait, arguments));
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
    // send the unbind queue frame
    return send(QueueUnbindFrame(_id, queue, exchange, routingkey, arguments));
}

/**
 *  Purge a queue
 *  @param  queue       queue to purge
 *  @param  flags       additional flags
 *  @return bool    
 */
bool ChannelImpl::purgeQueue(const std::string &name, int flags)
{
    // send the queue purge frame
    return send(QueuePurgeFrame(_id, name, flags & nowait));
}

/**
 *  Remove a queue
 *  @param  queue       queue to remove
 *  @param  flags       additional flags
 *  @return bool
 */
bool ChannelImpl::removeQueue(const std::string &name, int flags)
{
    // send the remove queue frame
    return send(QueueDeleteFrame(_id, name, flags & ifunused, flags & ifempty, flags & nowait));
}

/**
 *  Publish a message to an exchange
 * 
 *  The following flags can be used
 * 
 *      -   mandatory   if set, an unroutable message will be reported to the channel handler with the onReturned method
 *      -   immediate   if set, a message that could not immediately be consumed is returned to the onReturned method
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
    // we are going to send out multiple frames, each one will trigger a call to the handler,
    // which in turn could destruct the channel object, we need to monitor that
    Monitor monitor(this);
    
    // @todo do not copy the entire buffer to individual frames
    
    // send the publish frame
    if (!send(BasicPublishFrame(_id, exchange, routingKey, flags & mandatory, flags & immediate))) return false;
    
    // channel still valid?
    if (!monitor.valid()) return false;

    // send header
    if (!send(BasicHeaderFrame(_id, envelope))) return false;
    
    // channel and connection still valid?
    if (!monitor.valid() || !_connection) return false;
    
    // the max payload size is the max frame size minus the bytes for headers and trailer
    uint32_t maxpayload = _connection->maxPayload();
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
        if (!send(BodyFrame(_id, data + bytessent, chunksize))) return false;
        
        // channel still valid?
        if (!monitor.valid()) return false;
        
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
    return send(BasicQosFrame(_id, prefetchCount, false));
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
    return send(BasicConsumeFrame(_id, queue, tag, flags & nolocal, flags & noack, flags & exclusive, flags & nowait, arguments));
}

/**
 *  Cancel a running consumer
 *  @param  tag                 the consumer tag
 *  @param  flags               optional flags
 */
bool ChannelImpl::cancel(const std::string &tag, int flags)
{
    // send a cancel frame
    return send(BasicCancelFrame(_id, tag, flags & nowait));
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
    return send(BasicAckFrame(_id, deliveryTag, flags & multiple));
}

/**
 *  Reject a message
 *  @param  deliveryTag         the delivery tag
 *  @param  flags               optional flags
 *  @return bool
 */
bool ChannelImpl::reject(uint64_t deliveryTag, int flags)
{
    // send a nack frame
    return send(BasicNackFrame(_id, deliveryTag, flags & multiple, flags & requeue));
}

/**
 *  Recover un-acked messages
 *  @param  flags               optional flags
 *  @return bool
 */
bool ChannelImpl::recover(int flags)
{
    // send a nack frame
    return send(BasicRecoverFrame(_id, flags & requeue));
}

/**
 *  Send a frame over the channel
 *  @param  frame       frame to send
 *  @return bool        was the frame sent?
 */
bool ChannelImpl::send(const Frame &frame)
{
    // skip if channel is not connected
    if (_state != state_connected || !_connection) return false;
    
    // send to tcp connection
    return _connection->send(frame);
}

/**
 *  Report the received message
 */
void ChannelImpl::reportMessage()
{
    // skip if there is no message
    if (!_message) return;
    
    // after the report the channel may be destructed, monitor that
    Monitor monitor(this);
    
    // do we have a handler?
    if (_handler) _message->report(_parent, _handler);
    
    // skip if channel was destructed
    if (!monitor.valid()) return;
    
    // no longer need the message
    delete _message; 
    _message = nullptr;
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
    return _message = new ConsumedMessage(frame);
}

/**
 *  Create an incoming message
 *  @param  frame
 *  @return MessageImpl
 */
MessageImpl *ChannelImpl::message(const BasicReturnFrame &frame)
{
    // it should not be possible that a message already exists, but lets check it anyhow
    if (_message) delete _message;
    
    // construct a message
    return _message = new ReturnedMessage(frame);
}

/**
 *  End of namespace
 */
}

