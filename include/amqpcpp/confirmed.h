/**
 *  Confirmed.h
 *  
 *  A channel wrapper based on AMQP::Throttle that allows message callbacks to be installed
 *  on the publishes, to be called when they are confirmed by the message broker.
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
#include "deferredconfirmedpublish.h"
#include <memory>

/**
 *  Begin of namespaces
 */
namespace AMQP { 

/**
 *  Class definition
 */
class Confirmed : public Throttle, private Watchable
{
private:
    /**
     *  Set of open deliverytags. We want a normal set (not unordered_set) because
     *  removal will be cheaper for whole ranges.
     *  @var size_t
     */
    std::map<size_t, std::shared_ptr<DeferredConfirmedPublish>> _handlers;

    /**
     *  Called when the deliverytag(s) are acked/nacked
     *  @param  deliveryTag
     *  @param  multiple
     */
    virtual void onAck(uint64_t deliveryTag, bool multiple) override;
    virtual void onNack(uint64_t deliveryTag, bool multiple) override;

    /**
     *  Method that is called to report an error
     *  @param  message
     */
    virtual void reportError(const char *message) override;

public:
    /**
     *  Constructor
     *  @param  channel
     *  @param  throttle
     */
    Confirmed(AMQP::Channel &channel, size_t throttle) : Throttle(channel, throttle) {}

    /**
     *  Deleted copy constructor, deleted move constructor
     *  @param other
     */
    Confirmed(const Confirmed &other) = delete;
    Confirmed(Confirmed &&other) = delete;

    /**
     *  Deleted copy assignment, deleted move assignment
     *  @param  other
     */
    Confirmed &operator=(const Confirmed &other) = delete;
    Confirmed &operator=(Confirmed &&other) = delete;

    /**
     *  Virtual destructor
     */
    virtual ~Confirmed() = default;

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
     *  @return bool
     */
    DeferredConfirmedPublish &publish(const std::string &exchange, const std::string &routingKey, const Envelope &envelope, int flags = 0);
    DeferredConfirmedPublish &publish(const std::string &exchange, const std::string &routingKey, const std::string &message, int flags = 0) { return publish(exchange, routingKey, Envelope(message.data(), message.size()), flags); }
    DeferredConfirmedPublish &publish(const std::string &exchange, const std::string &routingKey, const char *message, size_t size, int flags = 0) { return publish(exchange, routingKey, Envelope(message, size), flags); }
    DeferredConfirmedPublish &publish(const std::string &exchange, const std::string &routingKey, const char *message, int flags = 0) { return publish(exchange, routingKey, Envelope(message, strlen(message)), flags); }
};

/**
 *  End of namespaces
 */
} 
