/**
 *  Tagger.h
 *  
 *  Base class that enables publisher confirms and keeps track of the sent 
 *  messages. You can wrap this class around a AMQP::Channel object and use
 *  this object for publishing instead. This is a base class that you cannot
 *  use directly. You should instead use:
 * 
 *  - Throttle: to throttle traffic to prevent flooding RabbitMQ
 *  - Reliable<Tagger>: to be notified about publish-confirms via callbacks
 *  - Reliable<Throttle>: to have throttle + notifications via callbacks
 *  
 *  @author Michael van der Werve <michael.vanderwerve@mailerq.com>
 *  @copyright 2020 Copernica BV
 */

/**
 *  Header guard
 */
#pragma once

/**
 *  Includes
 */
#include "deferredpublish.h"
#include <memory>

/**
 *  Begin of namespaces
 */
namespace AMQP { 

/**
 *  Class definition
 */
class Tagger : public Watchable
{
protected:
    /**
     *  The implementation for the channel
     *  @var    std::shared_ptr<ChannelImpl>
     */
    std::shared_ptr<ChannelImpl> _implementation;

    /**
     *  Current id, always starts at 1.
     *  @var uint64_t
     */
    uint64_t _current = 1;

    /**
     *  Deferred to set up on the close
     *  @var std::shared_ptr<Deferred>
     */
    std::shared_ptr<Deferred> _close;

    /**
     *  Callback to call when an error occurred
     *  @var ErrorCallback
     */
    ErrorCallback _errorCallback;


protected:
    /**
     *  Send method for a frame
     *  @param  id
     *  @param  frame
     */
    virtual bool send(uint64_t id, const Frame &frame);

    /**
     *  Method that is called to report an error.
     *  @param  message
     */
    virtual void reportError(const char *message);

    /**
     *  Method that gets called on ack/nack. If these methods are overridden, make sure 
     *  to also call the base class methods.
     *  @param  deliveryTag
     *  @param  multiple
     */
    virtual void onAck(uint64_t deliveryTag, bool multiple);
    virtual void onNack(uint64_t deliveryTag, bool multiple);

public:
    /**
     *  Constructor
     *  @param  channel
     */
    Tagger(AMQP::Channel &channel);

    /**
     *  Deleted copy constructor, deleted move constructor
     *  @param other
     */
    Tagger(const Tagger &other) = delete;
    Tagger(Tagger &&other) = delete;

    /**
     *  Deleted copy assignment, deleted move assignment
     *  @param  other
     */
    Tagger &operator=(const Tagger &other) = delete;
    Tagger &operator=(Tagger &&other) = delete;

    /**
     *  Virtual destructor
     */
    virtual ~Tagger();

    /**
     *  Method to check how many messages are still unacked.
     *  @return size_t
     */
    virtual size_t unacknowledged() const { return 0; }

    /**
     *  Publish a message to an exchange. See amqpcpp/channel.h for more details on the flags. 
     *  Delays actual publishing depending on the publisher confirms sent by RabbitMQ.
     * 
     *  @param  exchange    the exchange to publish to
     *  @param  routingkey  the routing key
     *  @param  envelope    the full envelope to send
     *  @param  message     the message to send
     *  @param  size        size of the message
     *  @param  flags       optional flags
     *  @return uint64_t
     */
    uint64_t publish(const std::string &exchange, const std::string &routingKey, const Envelope &envelope, int flags = 0);
    uint64_t publish(const std::string &exchange, const std::string &routingKey, const std::string &message, int flags = 0) { return publish(exchange, routingKey, Envelope(message.data(), message.size()), flags); }
    uint64_t publish(const std::string &exchange, const std::string &routingKey, const char *message, size_t size, int flags = 0) { return publish(exchange, routingKey, Envelope(message, size), flags); }
    uint64_t publish(const std::string &exchange, const std::string &routingKey, const char *message, int flags = 0) { return publish(exchange, routingKey, Envelope(message, strlen(message)), flags); }

    /**
     *  Close underlying channel
     *  @return Deferred&
     */
    Deferred &close();

    /**
     *  Install an error callback
     *  @param  callback
     */
    inline void onError(const ErrorCallback& callback) { return onError(ErrorCallback(callback)); }
    void onError(ErrorCallback&& callback);
};

/**
 *  End of namespaces
 */
} 
