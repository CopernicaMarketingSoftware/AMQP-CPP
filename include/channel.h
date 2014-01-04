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
     *  @param  queue       the target queue
     *  @param  exchange    the source exchange
     *  @param  routingkey  the routing key
     *  @param  flags       additional flags
     *  @param  arguments   additional bind arguments
     *  @return bool
     */
    bool bindQueue(const std::string &queue, const std::string &exchange, const std::string &routingkey, int flags, const Table &arguments) { return _implementation.bindQueue(exchange, queue, routingkey, flags, arguments); }
    bool bindQueue(const std::string &queue, const std::string &exchange, const std::string &routingkey, const Table &arguments) { return _implementation.bindQueue(exchange, queue, routingkey, 0, arguments); }
    bool bindQueue(const std::string &queue, const std::string &exchange, const std::string &routingkey, int flags = 0) { return _implementation.bindQueue(exchange, queue, routingkey, flags, Table()); }
    
    /**
     *  Unbind a queue from an exchange
     *  @param  queue       the target queue
     *  @param  exchange    the source exchange
     *  @param  routingkey  the routing key
     *  @param  arguments   additional bind arguments
     *  @return bool
     */
    bool unbindQueue(const std::string &queue, const std::string &exchange, const std::string &routingkey, const Table &arguments) {  return _implementation.unbindQueue(exchange, queue, routingkey, arguments); }
    bool unbindQueue(const std::string &queue, const std::string &exchange, const std::string &routingkey) { return _implementation.unbindQueue(exchange, queue, routingkey, Table()); }
    
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
     *  @todo   implement to onReturned() method
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

