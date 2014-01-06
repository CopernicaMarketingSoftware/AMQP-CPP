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
     *  @param  exchange
     *  @param  routingKey
     */
    Message(const std::string &exchange, const std::string &routingKey) :
        Envelope(nullptr, 0), _exchange(exchange), _routingKey(routingKey)
    {}
    
public:
    /**
     *  Destructor
     */
    virtual ~Message() {}

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

