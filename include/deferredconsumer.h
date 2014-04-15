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
class DeferredConsumer : public Deferred
{
private:
    /**
     *  Callback to execute when a message arrives
     *  @var    ConsumeCallbacl
     */
    ConsumeCallback _consumeCallback;

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
        if (_consumeCallback) _consumeCallback(message, deliveryTag, consumerTag, redelivered);
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
     *  @param  boolea  are we already failed?
     */
    DeferredConsumer(bool failed = false) : Deferred(failed) {}

public:
    /**
     *  Register a function to be called when a message arrives
     *
     *  Only one callback can be registered. Successive calls
     *  to this function will clear callbacks registered before.
     *
     *  @param  callback    the callback to execute
     */
    DeferredConsumer& onReceived(const ConsumeCallback &callback)
    {
        // store callback
        _consumeCallback = callback;
        return *this;
    }

    /**
     *  All the onSuccess() functions defined in the base class are accessible too
     */
    using Deferred::onSuccess;
};

/**
 *  End namespace
 */
}
