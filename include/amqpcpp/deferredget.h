/**
 *  DeferredGet.h
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 * 
 *  This class implements the 'shared_from_this' functionality, because
 *  it grabs a self-pointer when the callback is running, otherwise the onFinalize()
 *  is called before the actual message is consumed.
 */
class DeferredGet : public Deferred, public std::enable_shared_from_this<DeferredGet>
{
private:
    /**
     *  Pointer to the channel
     *  @var    ChannelImpl
     */
    ChannelImpl *_channel;

    /**
     *  Callback for incoming messages
     *  @var    MessageCallback
     */
    MessageCallback _messageCallback;

    /**
     *  Callback in case the queue is empty
     *  @var    EmptyCallback
     */
    EmptyCallback _emptyCallback;
    
    /**
     *  Callback with the number of messages still in the queue
     *  @var    SizeCallback
     */
    SizeCallback _sizeCallback;

    /**
     *  Report success when a message is indeed expected
     *  @param  count           number of messages in the queue
     *  @return Deferred
     */
    virtual const std::shared_ptr<Deferred> &reportSuccess(uint32_t messagecount) const override;

    /**
     *  Report success when queue was empty
     *  @return Deferred
     */
    virtual const std::shared_ptr<Deferred> &reportSuccess() const override;
    
    /**
     *  The channel implementation may call our
     *  private members and construct us
     */
    friend class ChannelImpl;
    friend class ConsumedMessage;

public:
    /**
     *  Protected constructor that can only be called
     *  from within the channel implementation
     *
     *  Note: this constructor _should_ be protected, but because make_shared
     *  will then not work, we have decided to make it public after all,
     *  because the work-around would result in not-so-easy-to-read code.
     * 
     *  @param  channel     the channel implementation
     *  @param  failed      are we already failed?
     */
    DeferredGet(ChannelImpl *channel, bool failed = false) : 
        Deferred(failed), _channel(channel) {}

public:
    /**
     *  Register a function to be called when a message arrives
     *  This fuction is also available as onReceived() and onMessage() because I always forget which name I gave to it
     *  @param  callback
     */
    DeferredGet &onSuccess(const MessageCallback &callback)
    {
        // store the callback
        _messageCallback = callback;
        
        // allow chaining
        return *this;
    }

    /**
     *  Register a function to be called when a message arrives
     *  This fuction is also available as onSuccess() and onMessage() because I always forget which name I gave to it
     *  @param  callback    the callback to execute
     */
    DeferredGet &onReceived(const MessageCallback &callback)
    {
        // store callback
        _messageCallback = callback;
        
        // allow chaining
        return *this;
    }

    /**
     *  Register a function to be called when a message arrives
     *  This fuction is also available as onSuccess() and onReceived() because I always forget which name I gave to it
     *  @param  callback    the callback to execute
     */
    DeferredGet &onMessage(const MessageCallback &callback)
    {
        // store callback
        _messageCallback = callback;
        
        // allow chaining
        return *this;
    }
    
    /**
     *  Register a function to be called if no message could be fetched
     *  @param  callback    the callback to execute
     */
    DeferredGet &onEmpty(const EmptyCallback &callback)
    {
        // store callback
        _emptyCallback = callback;
        
        // allow chaining
        return *this;
    }
    
    /**
     *  Register a function to be called when size information is known
     *  @param  callback    the callback to execute
     */
    DeferredGet &onSize(const SizeCallback &callback)
    {
        // store callback
        _sizeCallback = callback;
        
        // allow chaining
        return *this;
    }
};
    
/**
 *  End of namespace
 */
}

