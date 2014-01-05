/**
 *  Message.h
 *
 *  An incoming message has the same sort of information as an outgoing
 *  message, plus some additional information.
 *
 *  Message objects can not be constructed by end users, they are only constructed
 *  by the AMQP library, and passed to the ChannelHandler::onDelivered() method
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
class Message : public Envelope
{
protected:
    /**
     *  The consumer tag over which it was delivered
     *  @var    string
     */
    std::string _consumerTag;
    
    /**
     *  Unique delivery tag to identify and ack the mesage
     *  @var    uint64_t
     */
    uint64_t _deliveryTag;
    
    /**
     *  Is this a redelivered message / has it been delivered before?
     *  @var    bool
     */
    bool _redelivered;
    
    /**
     *  The exchange to which it was originally published
     *  @var    string
     */
    std::string _exchange;
    
    /**
     *  The routing key that was originally used
     *  @var    string
     */
    std::string _routingKey;
    
protected:
    /**
     *  The constructor is protected to ensure that endusers can not
     *  instantiate a message
     *  @param  consumerTag
     *  @param  deliveryTag
     *  @param  redelivered
     *  @param  exchange
     *  @param  routingKey
     */
    Message(const std::string &consumerTag, uint64_t deliveryTag, bool redelivered, const std::string &exchange, const std::string &routingKey) :
        Envelope(nullptr, 0), _consumerTag(consumerTag), _deliveryTag(deliveryTag), _redelivered(redelivered), _exchange(exchange), _routingKey(routingKey)
    {}
    
public:
    /**
     *  Destructor
     */
    virtual ~Message() {}

    /**
     *  The consumer tag over which it was delivered
     *  @return string
     */
    const std::string &consumerTag() const
    {
        return _consumerTag;
    }
    
    /**
     *  Unique delivery tag to identify and ack the mesage
     *  @return uint64_t
     */
    uint64_t deliveryTag() const
    {
        return _deliveryTag;
    }
    
    /**
     *  Is this a redelivered message / has it been delivered before?
     *  @var    bool
     */
    bool redelivered() const
    {
        return _redelivered;
    }
    
    /**
     *  The exchange to which it was originally published
     *  @var    string
     */
    const std::string &exchange() const
    {
        return _exchange;
    }
    
    /**
     *  The routing key that was originally used
     *  @var    string
     */
    const std::string &routingKey() const
    {
        return _routingKey;
    }
};

/**
 *  End of namespace
 */
}

