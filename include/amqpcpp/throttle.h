/**
 *  Throttle.h
 *  
 *  A channel wrapper that publishes more messages as soon as there is more capacity.
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
#include <cstdint>
#include <set>
#include <queue>
#include "copiedbuffer.h"
#include "channelimpl.h"

/**
 *  Begin of namespaces
 */
namespace AMQP {

/**
 *  Forward declarations
 */
class Channel; 

/**
 *  Class definition
 */
class Throttle
{
private:
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
     *  Last sent ID
     *  @var uint64_t
     */
    uint64_t _last = 0;

    /**
     *  Throttle
     *  @var size_t
     */
    size_t _throttle;

    /**
     *  Messages that should still be sent out.
     *  @var    queue
     */
    std::queue<std::pair<uint64_t, CopiedBuffer>> _queue;

    /**
     *  Set of open deliverytags. We want a normal set (not unordered_set) because
     *  removal will be cheaper for whole ranges.
     *  @var size_t
     */
    std::set<size_t> _open;

    /**
     *  Called when the deliverytag(s) are acked/nacked
     *  @param  deliveryTag
     *  @param  multiple
     */
    void onAck(uint64_t deliveryTag, bool multiple);

    /**
     *  Send method for a frame
     *  @param  id
     *  @param  frame
     */
    bool send(uint64_t id, const Frame &frame);

public:
    /**
     *  Constructor
     *  @param  channel 
     *  @param  throttle
     */
    Throttle(Channel &channel, size_t throttle);

    /**
     *  Deleted copy constructor, deleted move constructor
     *  @param other
     */
    Throttle(const Throttle &other) = delete;
    Throttle(Throttle &&other) = delete;

    /**
     *  Deleted copy assignment, deleted move assignment
     *  @param  other
     */
    Throttle &operator=(const Throttle &other) = delete;
    Throttle &operator=(Throttle &&other) = delete;

    /**
     *  Virtual destructor
     */
    virtual ~Throttle() = default;

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
    bool publish(const std::string &exchange, const std::string &routingKey, const Envelope &envelope, int flags = 0);
    bool publish(const std::string &exchange, const std::string &routingKey, const std::string &message, int flags = 0) { return publish(exchange, routingKey, Envelope(message.data(), message.size()), flags); }
    bool publish(const std::string &exchange, const std::string &routingKey, const char *message, size_t size, int flags = 0) { return publish(exchange, routingKey, Envelope(message, size), flags); }
    bool publish(const std::string &exchange, const std::string &routingKey, const char *message, int flags = 0) { return publish(exchange, routingKey, Envelope(message, strlen(message)), flags); }

    /**
     *  Get the number of messages that are waiting to be published
     *  @return uint64_t
     */
    size_t waiting() const { return _current - _last - 1; }

    /**
     *  Number of messages already sent but unacknowledged by rabbit
     *  @return size_t
     */
    size_t unacknowledged() const { return _open.size(); }

    /** 
     *  Get the throttle
     *  @return size_t
     */
    size_t throttle() const { return _throttle; }

    /**
     *  Set a new throttle. Note that this will only gradually take effect when set down, and
     *  the update is picked up on the next acknowledgement.
     *  @param  size_t
     */
    void throttle(size_t throttle) { _throttle = throttle; }
};

/**
 *  End of namespaces
 */
} 
