/**
 *  DeferredConsumer.h
 *
 *  Deferred callback for consumers
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  We extend from the default deferred and add extra functionality
 */
class DeferredConsumer : public Deferred<const std::string&>
{
private:
    /**
     *  Callback to execute when a message arrives
     */
    std::function<void(Channel *channel, const Message &message, uint64_t deliveryTag, const std::string &consumerTag, bool redelivered)> _messageCallback;

    /**
     *  Process a message
     *
     *  @param  message the message to process
     *  @param  deliveryTag the message delivery tag
     *  @param  consumerTag the tag we are consuming under
     *  @param  is this a redelivered message?
     */
    void message(const Message &message, uint64_t deliveryTag, const std::string &consumerTag, bool redelivered) const
    {
        // do we have a valid callback
        if (_messageCallback) _messageCallback(_channel, message, deliveryTag, consumerTag, redelivered);
    }

    /**
     *  The channel implementation may call our
     *  private members and construct us
     */
    friend class ChannelImpl;
    friend class ConsumedMessage;
protected:
    /**
     *  Protected constructor that can only be called
     *  from within the channel implementation
     *
     *  @param  channel the channel we operate under
     *  @param  boolea  are we already failed?
     */
    DeferredConsumer(Channel *channel, bool failed = false) :
        Deferred(channel, failed)
    {}
public:
    /**
     *  Register a function to be called when a message arrives
     *
     *  Only one callback can be registered. Successive calls
     *  to this function will clear callbacks registered before.
     *
     *  @param  callback    the callback to execute
     */
    DeferredConsumer& onReceived(const std::function<void(Channel *channel, const Message &message, uint64_t deliveryTag, const std::string &consumerTag, bool redelivered)>& callback)
    {
        // store callback
        _messageCallback = callback;
        return *this;
    }
};

/**
 *  End namespace
 */
}
