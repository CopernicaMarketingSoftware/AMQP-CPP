/**
 *  Class describing a (mid-level) AMQP channel implementation
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
class Channel
{
private:
    /**
     *  The implementation for the channel
     *  @var    ChannelImpl
     */
    ChannelImpl _implementation;

public:
    /**
     *  Construct a channel object
     *  @param  connection
     *  @param  handler
     */
    Channel(Connection *connection, ChannelHandler *handler) : _implementation(this, connection, handler) {}

    /**
     *  Destructor
     */
    virtual ~Channel() {}

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
    bool pause()
    {
        return _implementation.pause();
    }
    
    /**
     *  Resume a paused channel
     * 
     *  @return bool
     */
    bool resume()
    {
        return _implementation.resume();
    }

    /**
     *  Is the channel connected?
     *  @return bool
     */
    bool connected()
    {
        return _implementation.connected();
    }
    
    /**
     *  Start a transaction
     *  @return bool
     */
    bool startTransaction()
    {
        return _implementation.startTransaction();
    }
    
    /**
     *  Commit the current transaction
     *  @return bool
     */
    bool commitTransaction()
    {
        return _implementation.commitTransaction();
    }
    
    /**
     *  Rollback the current transaction
     *  @return bool
     */
    bool rollbackTransaction()
    {
        return _implementation.rollbackTransaction();
    }
    
    /**
     *  Declare an exchange
     *  
     *  If an empty name is supplied, a name will be assigned by the server.
     * 
     *  The following flags can be used for the exchange:
     *  
     *      -   durable     exchange survives a broker restart
     *      -   autodelete  exchange is automatically removed when all connected queues are removed
     *      -   passive     only check if the exchange exist
     * 
     *  @param  name        name of the exchange
     *  @param  type        exchange type
     *  @param  flags       exchange flags
     *  @param  arguments   additional arguments
     *  @return bool
     */
    bool declareExchange(const std::string &name, ExchangeType type, int flags, const Table &arguments) { return _implementation.declareExchange(name, type, flags, arguments); }
    bool declareExchange(const std::string &name, ExchangeType type, const Table &arguments) { return _implementation.declareExchange(name, type, 0, arguments); }
    bool declareExchange(const std::string &name, ExchangeType type = fanout, int flags = 0) { return _implementation.declareExchange(name, type, flags, Table()); }
    bool declareExchange(ExchangeType type, int flags, const Table &arguments) { return _implementation.declareExchange(std::string(), type, flags, arguments); }
    bool declareExchange(ExchangeType type, const Table &arguments) { return _implementation.declareExchange(std::string(), type, 0, arguments); }
    bool declareExchange(ExchangeType type = fanout, int flags = 0) { return _implementation.declareExchange(std::string(), type, flags, Table()); }
    
    /**
     *  Remove an exchange
     * 
     *  The following flags can be used for the exchange:
     *  
     *      -   ifunused    only delete if no queues are connected

     *  @param  name        name of the exchange to remove
     *  @param  flags       optional flags
     *  @return bool
     */
    bool removeExchange(const std::string &name, int flags = 0) { return _implementation.removeExchange(name, flags); }
    
    /**
     *  Bind two exchanges to each other
     * 
     *  The following flags can be used for the exchange
     * 
     *      -   nowait      do not wait on response
     * 
     *  @param  source      the source exchange
     *  @param  target      the target exchange
     *  @param  routingkey  the routing key
     *  @param  flags       optional flags
     *  @param  arguments   additional bind arguments
     *  @return bool
     */
    bool bindExchange(const std::string &source, const std::string &target, const std::string &routingkey, int flags, const Table &arguments) { return _implementation.bindExchange(source, target, routingkey, flags, arguments); }
    bool bindExchange(const std::string &source, const std::string &target, const std::string &routingkey, const Table &arguments) { return _implementation.bindExchange(source, target, routingkey, 0, arguments); }
    bool bindExchange(const std::string &source, const std::string &target, const std::string &routingkey, int flags = 0) { return _implementation.bindExchange(source, target, routingkey, flags, Table()); }
    
    /**
     *  Unbind two exchanges from one another
     * 
     *  The following flags can be used for the exchange
     * 
     *      -   nowait      do not wait on response
     * 
     *  @param  target      the target exchange
     *  @param  source      the source exchange
     *  @param  routingkey  the routing key
     *  @param  flags       optional flags
     *  @param  arguments   additional unbind arguments
     *  @return bool
     */
    bool unbindExchange(const std::string &target, const std::string &source, const std::string &routingkey, int flags, const Table &arguments) { return _implementation.unbindExchange(target, source, routingkey, flags, arguments); }
    bool unbindExchange(const std::string &target, const std::string &source, const std::string &routingkey, const Table &arguments) { return _implementation.unbindExchange(target, source, routingkey, 0, arguments); }
    bool unbindExchange(const std::string &target, const std::string &source, const std::string &routingkey, int flags = 0) { return _implementation.unbindExchange(target, source, routingkey, flags, Table()); }
    
    /**
     *  Declare a queue
     * 
     *  If you do not supply a name, a name will be assigned by the server.
     * 
     *  The flags can be a combination of the following values:
     * 
     *      -   durable     queue survives a broker restart
     *      -   autodelete  queue is automatically removed when all connected consumers are gone
     *      -   passive     only check if the queue exist
     *      -   exclusive   the queue only exists for this connection, and is automatically removed when connection is gone
     *    
     *  @param  name        name of the queue
     *  @param  flags       combination of flags
     *  @param  arguments   optional arguments
     */
    bool declareQueue(const std::string &name, int flags, const Table &arguments) { return _implementation.declareQueue(name, flags, arguments); }
    bool declareQueue(const std::string &name, const Table &arguments) { return _implementation.declareQueue(name, 0, arguments); }
    bool declareQueue(const std::string &name, int flags = 0) { return _implementation.declareQueue(name, flags, Table()); }
    bool declareQueue(int flags, const Table &arguments) { return _implementation.declareQueue(std::string(), flags, arguments); }
    bool declareQueue(const Table &arguments) { return _implementation.declareQueue(std::string(), 0, arguments); }
    bool declareQueue(int flags = 0) { return _implementation.declareQueue(std::string(), flags, Table()); }

    /**
     *  Bind a queue to an exchange
     *  
     *  The following flags can be used for the exchange
     * 
     *      -   nowait      do not wait on response
     * 
     *  @param  exchange    the source exchange
     *  @param  queue       the target queue
     *  @param  routingkey  the routing key
     *  @param  flags       additional flags
     *  @param  arguments   additional bind arguments
     *  @return bool
     */
    bool bindQueue(const std::string &exchange, const std::string &queue, const std::string &routingkey, int flags, const Table &arguments) { return _implementation.bindQueue(exchange, queue, routingkey, flags, arguments); }
    bool bindQueue(const std::string &exchange, const std::string &queue, const std::string &routingkey, const Table &arguments) { return _implementation.bindQueue(exchange, queue, routingkey, 0, arguments); }
    bool bindQueue(const std::string &exchange, const std::string &queue, const std::string &routingkey, int flags = 0) { return _implementation.bindQueue(exchange, queue, routingkey, flags, Table()); }
    
    /**
     *  Unbind a queue from an exchange
     *  @param  exchange    the source exchange
     *  @param  queue       the target queue
     *  @param  routingkey  the routing key
     *  @param  arguments   additional bind arguments
     *  @return bool
     */
    bool unbindQueue(const std::string &exchange, const std::string &queue, const std::string &routingkey, const Table &arguments) {  return _implementation.unbindQueue(exchange, queue, routingkey, arguments); }
    bool unbindQueue(const std::string &exchange, const std::string &queue, const std::string &routingkey) { return _implementation.unbindQueue(exchange, queue, routingkey, Table()); }
    
    /**
     *  Purge a queue
     * 
     *  The following flags can be used for the exchange
     * 
     *      -   nowait      do not wait on response
     * 
     *  @param  name        name of the queue
     *  @param  flags       additional flags
     *  @return bool
     */
    bool purgeQueue(const std::string &name, int flags = 0){ return _implementation.purgeQueue(name, flags); }
    
    /**
     *  Remove a queue
     * 
     *  The following flags can be used for the exchange:
     *  
     *      -   ifunused    only delete if no consumers are connected
     *      -   ifempty     only delete if the queue is empty
     *
     *  @param  name        name of the queue to remove
     *  @param  flags       optional flags
     *  @return bool
     */
    bool removeQueue(const std::string &name, int flags = 0) { return _implementation.removeQueue(name, flags); }
    
    /**
     *  Publish a message to an exchange
     * 
     *  The following flags can be used
     * 
     *      -   mandatory   if set, an unroutable message will be reported to the channel handler with the onReturned method
     *      -   immediate   if set, a message that could not immediately be consumed is returned to the onReturned method
     * 
     *  If either of the two flags is set, and the message could not immediately
     *  be published, the message is returned by the server to the client. If you
     *  want to catch such returned messages, you need to implement the 
     *  ChannelHandler::onReturned() method.
     * 
     *  @param  exchange    the exchange to publish to
     *  @param  routingkey  the routing key
     *  @param  flags       optional flags (see above)
     *  @param  envelope    the full envelope to send
     *  @param  message     the message to send
     *  @param  size        size of the message
     */
    bool publish(const std::string &exchange, const std::string &routingKey, int flags, const Envelope &envelope) { return _implementation.publish(exchange, routingKey, flags, envelope); }
    bool publish(const std::string &exchange, const std::string &routingKey, const Envelope &envelope) { return _implementation.publish(exchange, routingKey, 0, envelope); }
    bool publish(const std::string &exchange, const std::string &routingKey, int flags, const std::string &message) { return _implementation.publish(exchange, routingKey, flags, Envelope(message)); }
    bool publish(const std::string &exchange, const std::string &routingKey, const std::string &message) { return _implementation.publish(exchange, routingKey, 0, Envelope(message)); }
    bool publish(const std::string &exchange, const std::string &routingKey, int flags, const char *message, size_t size) { return _implementation.publish(exchange, routingKey, flags, Envelope(message, size)); }
    bool publish(const std::string &exchange, const std::string &routingKey, const char *message, size_t size) { return _implementation.publish(exchange, routingKey, 0, Envelope(message, size)); }
    
    /**
     *  Set the Quality of Service (QOS) for this channel
     * 
     *  When you consume messages, every single messages needs to be ack'ed to inform
     *  the RabbitMQ server that is has been received. The Qos setting specifies the
     *  number of unacked messages that may exist in the client application. The server
     *  stops delivering more messages if the number of unack'ed messages has reached
     *  the prefetchCount
     * 
     *  @param  prefetchCount       maximum number of messages to prefetch
     *  @return bool                whether the Qos frame is sent.
     */
    bool setQos(uint16_t prefetchCount)
    {
        return _implementation.setQos(prefetchCount);
    }
    
    /**
     *  Tell the RabbitMQ server that we're ready to consume messages
     * 
     *  After this method is called, RabbitMQ starts delivering messages to the client
     *  application. The consume tag is a string identifier that will be passed to
     *  each received message, so that you can associate incoming messages with a 
     *  consumer. If you do not specify a consumer tag, the server will assign one
     *  for you.
     * 
     *  The following flags are supported:
     * 
     *      -   nolocal             if set, messages published on this channel are not also consumed
     *      -   noack               if set, consumed messages do not have to be acked, this happens automatically
     *      -   exclusive           request exclusive access, only this consumer can access the queue
     *      -   nowait              the server does not have to send a response back that consuming is active
     * 
     *  The method ChannelHandler::onConsumerStarted() will be called when the 
     *  consumer has started (unless the nowait option was set, in which case
     *  no confirmation method is called)
     * 
     *  @param  queue               the queue from which you want to consume
     *  @param  tag                 a consumer tag that will be associated with this consume operation
     *  @param  flags               additional flags
     *  @param  arguments           additional arguments
     *  @return bool
     */
    bool consume(const std::string &queue, const std::string &tag, int flags, const Table &arguments) { return _implementation.consume(queue, tag, flags, arguments); }
    bool consume(const std::string &queue, const std::string &tag, int flags = 0) { return _implementation.consume(queue, tag, flags, Table()); }
    bool consume(const std::string &queue, const std::string &tag, const Table &arguments) { return _implementation.consume(queue, tag, 0, arguments); }
    bool consume(const std::string &queue, int flags, const Table &arguments) { return _implementation.consume(queue, std::string(), flags, arguments); }
    bool consume(const std::string &queue, int flags = 0) { return _implementation.consume(queue, std::string(), flags, Table()); }
    bool consume(const std::string &queue, const Table &arguments) { return _implementation.consume(queue, std::string(), 0, arguments); }
    
    /**
     *  Cancel a running consume call
     * 
     *  If you want to stop a running consumer, you can use this method with the consumer tag
     * 
     *  The following flags are supported:
     * 
     *      -   nowait              the server does not have to send a response back that the consumer has been cancelled
     * 
     *  The method ChannelHandler::onConsumerStopped() will be called when the consumer
     *  was succesfully stopped (unless the nowait option was used, in which case no
     *  confirmation method is called)
     * 
     *  @param  tag                 the consumer tag
     *  @param  flags               optional additional flags
     *  @return bool
     */
    bool cancel(const std::string &tag, int flags = 0) { return _implementation.cancel(tag, flags); }
    
    /**
     *  Acknoldge a received message
     * 
     *  When a message is received in the ChannelHandler::onReceived() method,
     *  you must acknoledge it so that RabbitMQ removes it from the queue (unless
     *  you are consuming with the noack option). This method can be used for
     *  this acknoledging.
     * 
     *  The following flags are supported:
     * 
     *      -   multiple            acknoledge multiple messages: all un-acked messages that were earlier delivered are acknowledged too
     * 
     *  @param  deliveryTag         the unique delivery tag of the message
     *  @param  flags               optional flags
     *  @return bool
     */
    bool ack(uint64_t deliveryTag, int flags=0) { return _implementation.ack(deliveryTag, flags); }
    
    /**
     *  Reject or nack a message
     * 
     *  When a message was received in the ChannelHandler::onReceived() method,
     *  and you don't want to acknoledge it, you can also choose to reject it by
     *  calling this reject method. 
     * 
     *  The following flags are supported:
     * 
     *      -   multiple            reject multiple messages: all un-acked messages that were earlier delivered are unacked too
     *      -   requeue             if set, the message is put back in the queue, otherwise it is dead-lettered/removed
     * 
     *  @param  deliveryTag         the unique delivery tag of the message
     *  @param  flags               optional flags
     *  @return bool
     */
    bool reject(uint64_t deliveryTag, int flags=0) { return _implementation.reject(deliveryTag, flags); }

    /**
     *  Recover all messages that were not yet acked
     * 
     *  This method asks the server to redeliver all unacknowledged messages on a specified 
     *  channel. Zero or more messages may be redelivered.
     * 
     *  The following flags are supported:
     * 
     *      -   requeue             if set, the server will requeue the messages, so the could also end up with at different consumer
     * 
     *  @param  flags
     *  @return bool
     */
    bool recover(int flags = 0) { return _implementation.recover(flags); }

    /**
     *  Close the current channel
     *  @return bool
     */
    bool close()
    {
        return _implementation.close();
    }

    /**
     *  Get the channel we're working on
     *  @return uint16_t
     */
    const uint16_t id() const
    {
        return _implementation.id();
    }
};

/**
 *  end namespace
 */
}

