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
#include "tagger.h"

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
class Throttle : public Tagger
{
protected:
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


protected:
    /**
     *  Send method for a frame
     *  @param  id
     *  @param  frame
     */
    virtual bool send(uint64_t id, const Frame &frame) override;

    /**
     *  Method that is called to report an error
     *  @param  message
     */
    virtual void reportError(const char *message) override;

    /**
     *  Method that is called to report an ack/nack
     *  @param  deliveryTag
     *  @param  multiple
     */
    virtual void onAck(uint64_t deliveryTag, bool multiple) override;
    virtual void onNack(uint64_t deliveryTag, bool multiple) override;

public:
    /**
     *  Constructor. Warning: this takes control of the channel, there should be no extra
     *  handlers set on the channel (onError) and no further publishes should be done on the
     *  raw channel either. Doing this will cause the throttle to work incorrectly, as the
     *  counters are not properly updated.
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
     *  Method to check how many messages are still unacked.
     *  @return size_t
     */
    virtual size_t unacknowledged() const override { return _open.size() + (_current - _last - 1); }

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

    /**
     *  Flush the throttle. This flushes it _without_ taking the throttle into account, e.g. the messages
     *  are sent in a burst over the channel.
     *  @param  max     optional maximum, 0 is flush all
     */
    size_t flush(size_t max = 0);
};

/**
 *  End of namespaces
 */
} 
