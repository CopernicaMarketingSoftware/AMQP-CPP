/**
 *  ReturnedMessage.h
 *
 *  Message that is received via a return call from the server, because it
 *  was published with the immediate or mandatory flag, and could not be
 *  delivered according to those rules.
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
class ReturnedMessage : public MessageImpl
{
private:
    /**
     *  The reply code
     *  @var    int16_t
     */
    int16_t _replyCode;
    
    /**
     *  The reply message
     *  @var    string
     */
    std::string _replyText;


public:
    /**
     *  Constructor
     *  @param  frame
     */
    ReturnedMessage(const BasicReturnFrame &frame) :
        MessageImpl(frame.exchange(), frame.routingKey()),
        _replyCode(frame.replyCode()), _replyText(frame.replyText()) {}
        
    /**
     *  Destructor
     */
    virtual ~ReturnedMessage() {}
    
    /**
     *  Report to the handler
     *  @param  channel
     *  @param  handler
     */
    virtual void report(Channel *channel, ChannelHandler *handler) override
    {
        // report to the handler
        handler->onReturned(channel, *this, _replyCode, _replyText);
    }
};

/**
 *  End of namespace
 */
}

