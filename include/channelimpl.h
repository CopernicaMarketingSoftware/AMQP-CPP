#pragma once
/**
 *  ChannelImpl.h
 *
 *  Extended channel object that is used internally by the library, but
 *  that has a private constructor so that it can not be used from outside
 *  the AMQP library
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class ChannelImpl : public Watchable
{
private:
    /**
     *  The actual channel object
     *  @var    Channel
     */
    Channel *_parent;

    /**
     *  Pointer to the connection
     *  @var    ConnectionImpl
     */
    ConnectionImpl *_connection;

    /**
     *  The handler that is notified about events
     *  @var    MyChannelHandler
     */
    ChannelHandler *_handler;

    /**
     *  Callback when the channel is ready
     */
    std::function<void(Channel *channel)> _readyCallback;

    /**
     *  Callback when the channel errors out
     */
    std::function<void(Channel *channel, const std::string& message)> _errorCallback;

    /**
     *  The callbacks waiting to be called
     */
    std::deque<Deferred<>> _callbacks;

    /**
     *  Callbacks with additional parameters
     */
    std::deque<Deferred<const std::string&, uint32_t, uint32_t>> _queueDeclaredCallbacks;
    std::deque<Deferred<uint32_t>> _queueRemovedCallbacks;

    /**
     *  The channel number
     *  @var uint16_t
     */
    uint16_t _id;

    /**
     *  State of the channel object
     *  @var enum
     */
    enum {
        state_connected,
        state_closing,
        state_closed
    } _state = state_connected;

    /**
     *  Is a transaction now active?
     *  @var bool
     */
    bool _transaction = false;

    /**
     *  The message that is now being received
     *  @var MessageImpl
     */
    MessageImpl *_message = nullptr;

    /**
     *  Construct a channel object
     *
     *  Note that the constructor is private, and that the Channel class is
     *  a friend. By doing this we ensure that nobody can instantiate this
     *  object, and that it can thus only be used inside the library.
     *
     *  @param  parent          the public channel object
     *  @param  connection      pointer to the connection
     *  @param  handler         handler that is notified on events
     */
    ChannelImpl(Channel *parent, Connection *connection, ChannelHandler *handler = nullptr);

public:
    /**
     *  Destructor
     */
    virtual ~ChannelImpl();

    /**
     *  Invalidate the channel
     *  This method is called when the connection is destructed
     */
    void invalidate()
    {
        _connection = nullptr;
    }

    /**
     *  Pause deliveries on a channel
     *
     *  This will stop all incoming messages
     *
     *  This function returns a deferred handler. Callbacks can be installed
     *  using onSuccess(), onError() and onFinalize() methods.
     */
    Deferred<>& pause();

    /**
     *  Resume a paused channel
     *
     *  This will resume incoming messages
     *
     *  This function returns a deferred handler. Callbacks can be installed
     *  using onSuccess(), onError() and onFinalize() methods.
     */
    Deferred<>& resume();

    /**
     *  Is the channel connected?
     *  @return bool
     */
    bool connected()
    {
        return _state == state_connected;
    }

    /**
     *  Start a transaction
     */
    Deferred<>& startTransaction();

    /**
     *  Commit the current transaction
     *
     *  This function returns a deferred handler. Callbacks can be installed
     *  using onSuccess(), onError() and onFinalize() methods.
     */
    Deferred<>& commitTransaction();

    /**
     *  Rollback the current transaction
     *
     *  This function returns a deferred handler. Callbacks can be installed
     *  using onSuccess(), onError() and onFinalize() methods.
     */
    Deferred<>& rollbackTransaction();

    /**
     *  declare an exchange
     *
     *  @param  name        name of the exchange to declare
     *  @param  type        type of exchange
     *  @param  flags       additional settings for the exchange
     *  @param  arguments   additional arguments
     *
     *  This function returns a deferred handler. Callbacks can be installed
     *  using onSuccess(), onError() and onFinalize() methods.
     */
    Deferred<>& declareExchange(const std::string &name, ExchangeType type, int flags, const Table &arguments);

    /**
     *  bind two exchanges

     *  @param  source      exchange which binds to target
     *  @param  target      exchange to bind to
     *  @param  routingKey  routing key
     *  @param  glags       additional flags
     *  @param  arguments   additional arguments for binding
     *
     *  This function returns a deferred handler. Callbacks can be installed
     *  using onSuccess(), onError() and onFinalize() methods.
     */
    Deferred<>& bindExchange(const std::string &source, const std::string &target, const std::string &routingkey, int flags, const Table &arguments);

    /**
     *  unbind two exchanges

     *  @param  source      the source exchange
     *  @param  target      the target exchange
     *  @param  routingkey  the routing key
     *  @param  flags       optional flags
     *  @param  arguments   additional unbind arguments
     *
     *  This function returns a deferred handler. Callbacks can be installed
     *  using onSuccess(), onError() and onFinalize() methods.
     */
    Deferred<>& unbindExchange(const std::string &source, const std::string &target, const std::string &routingkey, int flags, const Table &arguments);

    /**
     *  remove an exchange
     *
     *  @param  name        name of the exchange to remove
     *  @param  flags       additional settings for deleting the exchange
     *
     *  This function returns a deferred handler. Callbacks can be installed
     *  using onSuccess(), onError() and onFinalize() methods.
     */
    Deferred<>& removeExchange(const std::string &name, int flags);

    /**
     *  declare a queue
     *  @param  name        queue name
     *  @param  flags       additional settings for the queue
     *  @param  arguments   additional arguments
     *
     *  This function returns a deferred handler. Callbacks can be installed
     *  using onSuccess(), onError() and onFinalize() methods.
     */
    Deferred<const std::string&, uint32_t, uint32_t>& declareQueue(const std::string &name, int flags, const Table &arguments);

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
    Deferred<>& bindQueue(const std::string &exchangeName, const std::string &queueName, const std::string &routingkey, int flags, const Table &arguments);

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
    Deferred<>& unbindQueue(const std::string &exchangeName, const std::string &queueName, const std::string &routingkey, const Table &arguments);

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
    Deferred<uint32_t>& purgeQueue(const std::string &name, int flags);

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
    Deferred<uint32_t>& removeQueue(const std::string &name, int flags);

    /**
     *  Publish a message to an exchange
     *
     *  The following flags can be used
     *
     *      -   mandatory   if set, an unroutable message will be reported to the channel handler with the onReturned method
     *      -   immediate   if set, a message that could not immediately be consumed is returned to the onReturned method
     *
     *  If the mandatory or immediate flag is set, and the message could not immediately
     *  be published, the message will be returned to the client, and will eventually
     *  end up in your ChannelHandler::onReturned() method.
     *
     *  @param  exchange    the exchange to publish to
     *  @param  routingkey  the routing key
     *  @param  flags       optional flags (see above)
     *  @param  envelope    the full envelope to send
     *  @param  message     the message to send
     *  @param  size        size of the message
     */
    bool publish(const std::string &exchange, const std::string &routingKey, int flags, const Envelope &envelope);

    /**
     *  Set the Quality of Service (QOS) of the entire connection
     *  @param  prefetchCount       maximum number of messages to prefetch
     *
     *  This function returns a deferred handler. Callbacks can be installed
     *  using onSuccess(), onError() and onFinalize() methods.
     */
    Deferred<>& setQos(uint16_t prefetchCount);

    /**
     *  Tell the RabbitMQ server that we're ready to consume messages
     *  @param  queue               the queue from which you want to consume
     *  @param  tag                 a consumer tag that will be associated with this consume operation
     *  @param  flags               additional flags
     *  @param  arguments           additional arguments
     *  @return bool
     */
    bool consume(const std::string &queue, const std::string &tag, int flags, const Table &arguments);

    /**
     *  Cancel a running consumer
     *  @param  tag                 the consumer tag
     *  @param  flags               optional flags
     *  @return bool
     */
    bool cancel(const std::string &tag, int flags);

    /**
     *  Acknoledge a message
     *  @param  deliveryTag         the delivery tag
     *  @param  flags               optional flags
     *  @return bool
     */
    bool ack(uint64_t deliveryTag, int flags);

    /**
     *  Reject a message
     *  @param  deliveryTag         the delivery tag
     *  @param  flags               optional flags
     *  @return bool
     */
    bool reject(uint64_t deliveryTag, int flags);

    /**
     *  Recover messages that were not yet ack'ed
     *  @param  flags               optional flags
     *
     *  This function returns a deferred handler. Callbacks can be installed
     *  using onSuccess(), onError() and onFinalize() methods.
     */
    Deferred<>& recover(int flags);

    /**
     *  Close the current channel
     *
     *  This function returns a deferred handler. Callbacks can be installed
     *  using onSuccess(), onError() and onFinalize() methods.
     */
    Deferred<>& close();

    /**
     *  Get the channel we're working on
     *  @return uint16_t
     */
    const uint16_t id() const
    {
        return _id;
    }

    /**
     *  Send a frame over the channel
     *  @param  frame       frame to send
     *  @return bool        was frame succesfully sent?
     */
    bool send(const Frame &frame);

    /**
     *  Send a frame over the channel and
     *  get a deferred handler for it.
     *
     *  @param  frame       frame to send
     *  @param  message     the message to trigger if the frame cannot be send at all
     */
    Deferred<>& send(const Frame &frame, const char *message);

    /**
     *  Send a frame over the channel and
     *  get a deferred handler for it.
     *
     *  @param  frame       frame to send
     *  @param  message     the message to trigger if the frame cannot be send at all
     *  @param  queue       the queue to store the callbacks in
     */
    template <typename... Arguments>
    Deferred<Arguments...>& send(const Frame &frame, const char *message, std::deque<Deferred<Arguments...>>& queue);

    /**
     *  Report to the handler that the channel is closed
     */
    void reportClosed()
    {
        // change state
        _state = state_closed;

        // inform handler
        reportSuccess();
    }

    /**
     *  Report success
     *
     *  This function is called to report success for all
     *  cases where the callback does not receive any parameters
     */
    void reportSuccess()
    {
        // report success for the oldest request
        _callbacks.front().success();
        _callbacks.pop_front();
    }

    /**
     *  Report to the handler that the channel is opened
     */
    void reportReady()
    {
        // inform handler
        if (_readyCallback) _readyCallback(_parent);
    }

    /**
     *  Report an error message on a channel
     *  @param  message
     */
    void reportError(const std::string &message)
    {
        // change state
        _state = state_closed;

        // inform handler
        if (_errorCallback) _errorCallback(_parent, message);

        // and all waiting deferred callbacks
        for (auto &deferred : _callbacks)               deferred.error(message);
        for (auto &deferred : _queueDeclaredCallbacks)  deferred.error(message);
        for (auto &deferred : _queueRemovedCallbacks)   deferred.error(message);
    }

    /**
     *  Report that the queue was succesfully declared
     *  @param  queueName       name of the queue which was declared
     *  @param  messagecount    number of messages currently in the queue
     *  @param  consumerCount   number of active consumers in the queue
     */
    void reportQueueDeclared(const std::string &queueName, uint32_t messageCount, uint32_t consumerCount)
    {
        // report success for the oldest queue declare callbacks
        _queueDeclaredCallbacks.front().success(queueName, messageCount, consumerCount);
        _queueDeclaredCallbacks.pop_front();
    }

    /**
     *  Report that a queue was succesfully deleted
     *  @param  messageCount    number of messages left in queue, now deleted
     */
    void reportQueueDeleted(uint32_t messageCount)
    {
        // report success for the oldest queue remove callbacks
        _queueRemovedCallbacks.front().success(messageCount);
        _queueRemovedCallbacks.pop_front();
    }

    /**
     *  Report that a queue was succesfully purged
     *  @param  messageCount    number of messages purged
     */
    void reportQueuePurged(uint32_t messageCount)
    {
        // report success for the oldest queue remove callbacks
        _queueRemovedCallbacks.front().success(messageCount);
        _queueRemovedCallbacks.pop_front();
    }

    /**
     *  Report that a consumer has started
     *  @param  tag     the consumer tag
     */
    void reportConsumerStarted(const std::string &tag)
    {
        if (_handler) _handler->onConsumerStarted(_parent, tag);
    }

    /**
     *  Report that a consumer has stopped
     *  @param  tag     the consumer tag
     */
    void reportConsumerStopped(const std::string &tag)
    {
        if (_handler) _handler->onConsumerStopped(_parent, tag);
    }

    /**
     *  Report that a message was received
     */
    void reportMessage();

    /**
     *  Create an incoming message
     *  @param  frame
     *  @return MessageImpl
     */
    MessageImpl *message(const BasicDeliverFrame &frame);
    MessageImpl *message(const BasicReturnFrame &frame);

    /**
     *  Retrieve the current incoming message
     *  @return MessageImpl
     */
    MessageImpl *message()
    {
        return _message;
    }

    /**
     *  The channel class is its friend, thus can it instantiate this object
     */
    friend class Channel;

};

/**
 *  End of namespace
 */
}

