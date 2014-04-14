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
ChannelImpl::ChannelImpl(Channel *parent, Connection *connection) :
    _parent(parent),
    _connection(&connection->_implementation)
{
    // add the channel to the connection
    _id = _connection->add(this);

    // check if the id is valid
    if (_id == 0)
    {
        // this is invalid
        _state = state_closed;
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
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred<>& ChannelImpl::pause()
{
    // send a channel flow frame
    return send<>(ChannelFlowFrame(_id, false), "Cannot send channel flow frame");
}

/**
 *  Resume a paused channel
 *
 *  This will resume incoming messages
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred<>& ChannelImpl::resume()
{
    // send a channel flow frame
    return send<>(ChannelFlowFrame(_id, true), "Cannot send channel flow frame");
}

/**
 *  Start a transaction
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred<>& ChannelImpl::startTransaction()
{
    // send a transaction frame
    return send<>(TransactionSelectFrame(_id), "Cannot send transaction start frame");
}

/**
 *  Commit the current transaction
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred<>& ChannelImpl::commitTransaction()
{
    // send a transaction frame
    return send<>(TransactionCommitFrame(_id), "Cannot send transaction commit frame");
}

/**
 *  Rollback the current transaction
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred<>& ChannelImpl::rollbackTransaction()
{
    // send a transaction frame
    return send<>(TransactionRollbackFrame(_id), "Cannot send transaction commit frame");
}

/**
 *  Close the current channel
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred<>& ChannelImpl::close()
{
    // channel could be dead after send operation, we need to monitor that
    Monitor monitor(this);

    // send a channel close frame
    auto &handler = send<>(ChannelCloseFrame(_id), "Cannot send channel close frame");

    // was the frame sent and are we still alive?
    if (handler && monitor.valid()) _state = state_closing;

    // done
    return handler;
}

/**
 *  declare an exchange

 *  @param  name        name of the exchange to declare
 *  @param  type        type of exchange
 *  @param  flags       additional settings for the exchange
 *  @param  arguments   additional arguments
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred<>& ChannelImpl::declareExchange(const std::string &name, ExchangeType type, int flags, const Table &arguments)
{
    // convert exchange type
    std::string exchangeType;
    if (type == ExchangeType::fanout) exchangeType = "fanout";
    if (type == ExchangeType::direct) exchangeType = "direct";
    if (type == ExchangeType::topic)  exchangeType = "topic";
    if (type == ExchangeType::headers)exchangeType = "headers";

    // send declare exchange frame
    return send<>(ExchangeDeclareFrame(_id, name, exchangeType, flags & passive, flags & durable, flags & nowait, arguments), "Cannot send exchange declare frame");
}

/**
 *  bind an exchange
 *
 *  @param  source      exchange which binds to target
 *  @param  target      exchange to bind to
 *  @param  routingKey  routing key
 *  @param  flags       additional flags
 *  @param  arguments   additional arguments for binding
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred<>& ChannelImpl::bindExchange(const std::string &source, const std::string &target, const std::string &routingkey, int flags, const Table &arguments)
{
    // send exchange bind frame
    return send<>(ExchangeBindFrame(_id, target, source, routingkey, flags & nowait, arguments), "Cannot send exchange bind frame");
}

/**
 *  unbind two exchanges
 *
 *  @param  source      the source exchange
 *  @param  target      the target exchange
 *  @param  routingkey  the routing key
 *  @param  flags       optional flags
 *  @param  arguments   additional unbind arguments
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred<>& ChannelImpl::unbindExchange(const std::string &source, const std::string &target, const std::string &routingkey, int flags, const Table &arguments)
{
    // send exchange unbind frame
    return send<>(ExchangeUnbindFrame(_id, target, source, routingkey, flags & nowait, arguments), "Cannot send exchange unbind frame");
}

/**
 *  remove an exchange
 *
 *  @param  name        name of the exchange to remove
 *  @param  flags       additional settings for deleting the exchange
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred<>& ChannelImpl::removeExchange(const std::string &name, int flags)
{
    // send delete exchange frame
    return send<>(ExchangeDeleteFrame(_id, name, flags & ifunused, flags & nowait), "Cannot send exchange delete frame");
}

/**
 *  declare a queue
 *  @param  name        queue name
 *  @param  flags       additional settings for the queue
 *  @param  arguments   additional arguments
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred<const std::string&, uint32_t, uint32_t>& ChannelImpl::declareQueue(const std::string &name, int flags, const Table &arguments)
{
    // send the queuedeclareframe
    return send<const std::string&, uint32_t, uint32_t>(QueueDeclareFrame(_id, name, flags & passive, flags & durable, flags & exclusive, flags & autodelete, flags & nowait, arguments), "Cannot send queue declare frame");
}

/**
 *  Bind a queue to an exchange
 *
 *  @param  exchangeName    name of the exchange to bind to
 *  @param  queueName       name of the queue
 *  @param  routingkey      routingkey
 *  @param  flags           additional flags
 *  @param  arguments       additional arguments
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred<>& ChannelImpl::bindQueue(const std::string &exchangeName, const std::string &queueName, const std::string &routingkey, int flags, const Table &arguments)
{
    // send the bind queue frame
    return send<>(QueueBindFrame(_id, queueName, exchangeName, routingkey, flags & nowait, arguments), "Cannot send queue bind frame");
}

/**
 *  Unbind a queue from an exchange
 *
 *  @param  exchange    the source exchange
 *  @param  queue       the target queue
 *  @param  routingkey  the routing key
 *  @param  arguments   additional bind arguments
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred<>& ChannelImpl::unbindQueue(const std::string &exchange, const std::string &queue, const std::string &routingkey, const Table &arguments)
{
    // send the unbind queue frame
    return send<>(QueueUnbindFrame(_id, queue, exchange, routingkey, arguments), "Cannot send queue unbind frame");
}

/**
 *  Purge a queue
 *  @param  queue       queue to purge
 *  @param  flags       additional flags
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 *
 *  The onSuccess() callback that you can install should have the following signature:
 *
 *      void myCallback(AMQP::Channel *channel, uint32_t messageCount);
 *
 *  For example: channel.declareQueue("myqueue").onSuccess([](AMQP::Channel *channel, uint32_t messageCount) {
 *
 *      std::cout << "Queue purged, all " << messageCount << " messages removed" << std::endl;
 *
 *  });
 */
Deferred<uint32_t>& ChannelImpl::purgeQueue(const std::string &name, int flags)
{
    // send the queue purge frame
    return send<uint32_t>(QueuePurgeFrame(_id, name, flags & nowait), "Cannot send queue purge frame");
}

/**
 *  Remove a queue
 *  @param  queue       queue to remove
 *  @param  flags       additional flags
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 *
 *  The onSuccess() callback that you can install should have the following signature:
 *
 *      void myCallback(AMQP::Channel *channel, uint32_t messageCount);
 *
 *  For example: channel.declareQueue("myqueue").onSuccess([](AMQP::Channel *channel, uint32_t messageCount) {
 *
 *      std::cout << "Queue deleted, along with " << messageCount << " messages" << std::endl;
 *
 *  });
 */
Deferred<uint32_t>& ChannelImpl::removeQueue(const std::string &name, int flags)
{
    // send the remove queue frame
    return send<uint32_t>(QueueDeleteFrame(_id, name, flags & ifunused, flags & ifempty, flags & nowait), "Cannot send remove queue frame");
}

/**
 *  Publish a message to an exchange
 *
 *  @param  exchange    the exchange to publish to
 *  @param  routingkey  the routing key
 *  @param  envelope    the full envelope to send
 *  @param  message     the message to send
 *  @param  size        size of the message
 */
bool ChannelImpl::publish(const std::string &exchange, const std::string &routingKey, const Envelope &envelope)
{
    // we are going to send out multiple frames, each one will trigger a call to the handler,
    // which in turn could destruct the channel object, we need to monitor that
    Monitor monitor(this);

    // @todo do not copy the entire buffer to individual frames

    // send the publish frame
    if (!send(BasicPublishFrame(_id, exchange, routingKey))) return false;

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
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred<>& ChannelImpl::setQos(uint16_t prefetchCount)
{
    // send a qos frame
    return send(BasicQosFrame(_id, prefetchCount, false), "Cannot send basic QOS frame");
}

/**
 *  Tell the RabbitMQ server that we're ready to consume messages
 *  @param  queue               the queue from which you want to consume
 *  @param  tag                 a consumer tag that will be associated with this consume operation
 *  @param  flags               additional flags
 *  @param  arguments           additional arguments
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 *
 *  The onSuccess() callback that you can install should have the following signature:
 *
 *      void myCallback(AMQP::Channel *channel, const std::string& tag);
 *
 *  For example: channel.declareQueue("myqueue").onSuccess([](AMQP::Channel *channel, const std::string& tag) {
 *
 *      std::cout << "Started consuming under tag " << tag << std::endl;
 *
 *  });
 */
DeferredConsumer& ChannelImpl::consume(const std::string &queue, const std::string &tag, int flags, const Table &arguments)
{
    // create the deferred consumer
    _consumer = std::unique_ptr<DeferredConsumer>(new DeferredConsumer(_parent, false));

    // can we send the basic consume frame?
    if (!send(BasicConsumeFrame(_id, queue, tag, flags & nolocal, flags & noack, flags & exclusive, flags & nowait, arguments)))
    {
        // we set the consumer to be failed immediately
        _consumer->_failed = true;

        // we should call the error function later
        // TODO
    }

    // return the consumer
    return *_consumer;
}

/**
 *  Cancel a running consumer
 *  @param  tag                 the consumer tag
 *  @param  flags               optional flags
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 *
 *  The onSuccess() callback that you can install should have the following signature:
 *
 *      void myCallback(AMQP::Channel *channel, const std::string& tag);
 *
 *  For example: channel.declareQueue("myqueue").onSuccess([](AMQP::Channel *channel, const std::string& tag) {
 *
 *      std::cout << "Started consuming under tag " << tag << std::endl;
 *
 *  });
 */
Deferred<const std::string&>& ChannelImpl::cancel(const std::string &tag, int flags)
{
    // send a cancel frame
    return send<const std::string&>(BasicCancelFrame(_id, tag, flags & nowait), "Cannot send basic cancel frame");
}

/**
 *  Acknowledge a message
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
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred<>& ChannelImpl::recover(int flags)
{
    // send a nack frame
    return send(BasicRecoverFrame(_id, flags & requeue), "Cannot send basic recover frame");
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
 *  Send a frame over the channel and get a deferred handler for it.
 *
 *  @param  frame       frame to send
 *  @param  message     the message to trigger if the frame cannot be send at all
 */
template <typename... Arguments>
Deferred<Arguments...>& ChannelImpl::send(const Frame &frame, const char *message)
{
    // create a new deferred handler and get a pointer to it
    // note: cannot use auto here or the lambda below chokes
    // when compiling under gcc 4.8
    Deferred<Arguments...> *handler = &_callbacks.push_back(Deferred<Arguments...>(_parent));

    // send the frame over the channel
    if (!send(frame))
    {
        // we can immediately put the handler in failed state
        handler->_failed = true;

        // register an error on the deferred handler
        // after a timeout, so it gets called only
        // after a possible handler was installed.
        _connection->_handler->setTimeout(_connection->_parent, 0, [handler, message]() {
            // emit an error on the handler
            handler->error(message);
        });
    }

    // return the new handler
    return *handler;
}

/**
 *  Report the received message
 */
void ChannelImpl::reportMessage()
{
    // skip if there is no message
    if (!_message) return;

    // do we even have a consumer?
    if (!_consumer)
    {
        // this should not be possible: receiving a message without doing a consume() call
        reportError("Received message without having a consumer");
    }
    else
    {
        // after the report the channel may be destructed, monitor that
        Monitor monitor(this);

        // send message to the consumer
        _message->report(*_consumer);

        // skip if channel was destructed
        if (!monitor.valid()) return;
    }

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
 *  End of namespace
 */
}

