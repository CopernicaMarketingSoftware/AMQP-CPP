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
#include "basicrejectframe.h"

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

    // destruct deferred results
    while (_oldestCallback) _oldestCallback.reset(_oldestCallback->next());
}

/**
 *  Push a deferred result
 *  @param  result          The deferred object to push
 */
Deferred &ChannelImpl::push(Deferred *deferred)
{
    // do we already have an oldest?
    if (!_oldestCallback) _oldestCallback.reset(deferred);

    // do we already have a newest?
    if (_newestCallback) _newestCallback->add(deferred);

    // store newest callback
    _newestCallback = deferred;

    // done
    return *deferred;
}

/**
 *  Send a frame and push a deferred result
 *  @param  frame           The frame to send
 */
Deferred &ChannelImpl::push(const Frame &frame)
{
    // send the frame, and push the result
    return push(new Deferred(send(frame)));
}

/**
 *  Pause deliveries on a channel
 *
 *  This will stop all incoming messages
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred &ChannelImpl::pause()
{
    // send a channel flow frame
    return push(ChannelFlowFrame(_id, false));
}

/**
 *  Resume a paused channel
 *
 *  This will resume incoming messages
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred &ChannelImpl::resume()
{
    // send a channel flow frame
    return push(ChannelFlowFrame(_id, true));
}

/**
 *  Start a transaction
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred &ChannelImpl::startTransaction()
{
    // send a transaction frame
    return push(TransactionSelectFrame(_id));
}

/**
 *  Commit the current transaction
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred &ChannelImpl::commitTransaction()
{
    // send a transaction frame
    return push(TransactionCommitFrame(_id));
}

/**
 *  Rollback the current transaction
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred &ChannelImpl::rollbackTransaction()
{
    // send a transaction frame
    return push(TransactionRollbackFrame(_id));
}

/**
 *  Close the current channel
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred &ChannelImpl::close()
{
    // send a channel close frame
    auto &handler = push(ChannelCloseFrame(_id));

    // was the frame sent and are we still alive?
    if (handler) _state = state_closing;

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
Deferred &ChannelImpl::declareExchange(const std::string &name, ExchangeType type, int flags, const Table &arguments)
{
    // convert exchange type
    std::string exchangeType;
    if (type == ExchangeType::fanout) exchangeType = "fanout";
    if (type == ExchangeType::direct) exchangeType = "direct";
    if (type == ExchangeType::topic)  exchangeType = "topic";
    if (type == ExchangeType::headers)exchangeType = "headers";

    // send declare exchange frame
    return push(ExchangeDeclareFrame(_id, name, exchangeType, flags & passive, flags & durable, false, arguments));
}

/**
 *  bind an exchange
 *
 *  @param  source      exchange which binds to target
 *  @param  target      exchange to bind to
 *  @param  routingKey  routing key
 *  @param  arguments   additional arguments for binding
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred &ChannelImpl::bindExchange(const std::string &source, const std::string &target, const std::string &routingkey, const Table &arguments)
{
    // send exchange bind frame
    return push(ExchangeBindFrame(_id, target, source, routingkey, false, arguments));
}

/**
 *  unbind two exchanges
 *
 *  @param  source      the source exchange
 *  @param  target      the target exchange
 *  @param  routingkey  the routing key
 *  @param  arguments   additional unbind arguments
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred &ChannelImpl::unbindExchange(const std::string &source, const std::string &target, const std::string &routingkey, const Table &arguments)
{
    // send exchange unbind frame
    return push(ExchangeUnbindFrame(_id, target, source, routingkey, false, arguments));
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
Deferred &ChannelImpl::removeExchange(const std::string &name, int flags)
{
    // send delete exchange frame
    return push(ExchangeDeleteFrame(_id, name, flags & ifunused, false));
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
DeferredQueue &ChannelImpl::declareQueue(const std::string &name, int flags, const Table &arguments)
{
    // the frame to send
    QueueDeclareFrame frame(_id, name, flags & passive, flags & durable, flags & exclusive, flags & autodelete, false, arguments);

    // send the queuedeclareframe
    auto *result = new DeferredQueue(send(frame));

    // add the deferred result
    push(result);

    // done
    return *result;
}

/**
 *  Bind a queue to an exchange
 *
 *  @param  exchangeName    name of the exchange to bind to
 *  @param  queueName       name of the queue
 *  @param  routingkey      routingkey
 *  @param  arguments       additional arguments
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred &ChannelImpl::bindQueue(const std::string &exchangeName, const std::string &queueName, const std::string &routingkey, const Table &arguments)
{
    // send the bind queue frame
    return push(QueueBindFrame(_id, queueName, exchangeName, routingkey, false, arguments));
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
Deferred &ChannelImpl::unbindQueue(const std::string &exchange, const std::string &queue, const std::string &routingkey, const Table &arguments)
{
    // send the unbind queue frame
    return push(QueueUnbindFrame(_id, queue, exchange, routingkey, arguments));
}

/**
 *  Purge a queue
 *  @param  queue       queue to purge
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
DeferredDelete &ChannelImpl::purgeQueue(const std::string &name)
{
    // the frame to send
    QueuePurgeFrame frame(_id, name, false);

    // send the frame, and create deferred object
    auto *deferred = new DeferredDelete(send(frame));

    // push to list
    push(deferred);

    // done
    return *deferred;
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
DeferredDelete &ChannelImpl::removeQueue(const std::string &name, int flags)
{
    // the frame to send
    QueueDeleteFrame frame(_id, name, flags & ifunused, flags & ifempty, false);

    // send the frame, and create deferred object
    auto *deferred = new DeferredDelete(send(frame));

    // push to list
    push(deferred);

    // done
    return *deferred;
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
Deferred &ChannelImpl::setQos(uint16_t prefetchCount)
{
    // send a qos frame
    return push(BasicQosFrame(_id, prefetchCount, false));
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
    // the frame to send
    BasicConsumeFrame frame(_id, queue, tag, flags & nolocal, flags & noack, flags & exclusive, false, arguments);

    // send the frame, and create deferred object
    auto *deferred = new DeferredConsumer(this, send(frame));

    // push to list
    push(deferred);

    // done
    return *deferred;
}

/**
 *  Cancel a running consumer
 *  @param  tag                 the consumer tag
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
DeferredCancel &ChannelImpl::cancel(const std::string &tag)
{
    // the cancel frame to send
    BasicCancelFrame frame(_id, tag, false);

    // send the frame, and create deferred object
    auto *deferred = new DeferredCancel(this, send(frame));

    // push to list
    push(deferred);

    // done
    return *deferred;
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
    // should we reject multiple messages?
    if (flags & multiple)
    {
        // send a nack frame
        return send(BasicNackFrame(_id, deliveryTag, true, flags & requeue));
    }
    else
    {
        // send a reject frame
        return send(BasicRejectFrame(_id, deliveryTag, flags & requeue));
    }
}

/**
 *  Recover un-acked messages
 *  @param  flags               optional flags
 *
 *  This function returns a deferred handler. Callbacks can be installed
 *  using onSuccess(), onError() and onFinalize() methods.
 */
Deferred &ChannelImpl::recover(int flags)
{
    // send a nack frame
    return push(BasicRecoverFrame(_id, flags & requeue));
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

    // are we currently in synchronous mode or are there
    // other frames waiting for their turn to be sent?
    if (_synchronous || !_queue.empty())
    {
        // we need to wait until the synchronous frame has
        // been processed, so queue the frame until it was
        _queue.emplace(frame.synchronous(), frame.buffer());

        // it was of course not actually sent but we pretend
        // that it was, because no error occured
        return true;
    }

    // enter synchronous mode if necessary
    _synchronous = frame.synchronous();

    // send to tcp connection
    return _connection->send(frame);
}

/**
 *  Signal the channel that a synchronous operation
 *  was completed. After this operation, waiting
 *  frames can be sent out.
 */
void ChannelImpl::synchronized()
{
    // we are no longer waiting for synchronous operations
    _synchronous = false;

    // we need to monitor the channel for validity
    Monitor monitor(this);

    // send all frames while not in synchronous mode
    while (monitor.valid() && !_synchronous && !_queue.empty())
    {
        // retrieve the first buffer and synchronous
        auto pair = std::move(_queue.front());

        // remove from the list
        _queue.pop();

        // mark as synchronous if necessary
        _synchronous = pair.first;

        // send it over the connection
        _connection->send(std::move(pair.second));
    }
}

/**
 *  Report the received message
 */
void ChannelImpl::reportMessage()
{
    // skip if there is no message
    if (!_message) return;

    // look for the consumer
    auto iter = _consumers.find(_message->consumer());
    if (iter == _consumers.end()) return;

    // is this a valid callback method
    if (!iter->second) return;

    // after the report the channel may be destructed, monitor that
    Monitor monitor(this);

    // call the callback
    _message->report(iter->second);

    // skip if channel was destructed
    if (!monitor.valid()) return;

    // no longer need the message
    delete _message; _message = nullptr;
}

/**
 *  Create an incoming message
 *  @param  frame
 *  @return ConsumedMessage
 */
ConsumedMessage *ChannelImpl::message(const BasicDeliverFrame &frame)
{
    // destruct if message is already set
    if (_message) delete _message;

    // construct a message
    return _message = new ConsumedMessage(frame);
}

/**
 *  End of namespace
 */
}

