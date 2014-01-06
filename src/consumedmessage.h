/**
 *  Base class for a message implementation
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
class ConsumedMessage : public MessageImpl
{
private:
    /**
     *  The consumer tag
     *  @var string
     */
    std::string _consumerTag;

    /**
     *  The delivery tag
     *  @var uint64_t
     */
    uint64_t _deliveryTag;
    
    /**
     *  Is this a redelivered message?
     *  @var bool
     */
    bool _redelivered;


public:
    /**
     *  Constructor
     *  @param  frame
     */
    ConsumedMessage(const BasicDeliverFrame &frame) : 
        MessageImpl(frame.exchange(), frame.routingKey()), 
        _consumerTag(frame.consumerTag()), _deliveryTag(frame.deliveryTag()), _redelivered(frame.redelivered())
    {}
        
    /**
     *  Destructor
     */
    virtual ~ConsumedMessage() {}
    
    /**
     *  Report to the handler
     *  @param  channel
     *  @param  handler
     */
    virtual void report(Channel *channel, ChannelHandler *handler) override
    {
        // report to the handler
        handler->onReceived(channel, *this, _deliveryTag, _consumerTag, _redelivered);
    }
};

/**
 *  End of namespace
 */
}

