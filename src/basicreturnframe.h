/**
 *  Class describing a basic return frame
 * 
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace AMQP{

/**
 *  Class implementation
 */
class BasicReturnFrame : public BasicFrame {
private:
    /**
     *  reply code
     *  @var int16_t
     */
    int16_t _replyCode;

    /**
     *  reply text
     *  @var ShortString
     */
    ShortString _replyText;

    /**
     *  the name of the exchange to publish to. An empty exchange name means the default exchange.
     *  @var ShortString
     */
    ShortString _exchange;

    /**
     *  Message routing key
     *  @var ShortString
     */
    ShortString _routingKey;

protected:
    /**
     *  Encode a frame on a string buffer
     *
     *  @param   buffer  buffer to write frame to
     */
    virtual void fill(OutBuffer& buffer) const override
    {
        // call base
        BasicFrame::fill(buffer);

        // add fields
        buffer.add(_replyCode);
        _replyText.fill(buffer);
        _exchange.fill(buffer);
        _routingKey.fill(buffer);
    }

public:
    /**
     *  Construct a basic return frame
     *
     *  @param  channel         channel identifier
     *  @param  replyCode       reply code
     *  @param  replyText       reply text                      
     *  @param  exchange        name of exchange to publish to   
     *  @param  routingKey      message routing key
     */
    BasicReturnFrame(uint16_t channel, int16_t replyCode, const std::string& replyText = "", const std::string& exchange = "", const std::string& routingKey = "") :
        BasicFrame(channel, replyText.length() + exchange.length() + routingKey.length() + 5), // 3 for each string (extra size byte), 2 for uint16_t
        _replyCode(replyCode),
        _replyText(replyText),
        _exchange(exchange),
        _routingKey(routingKey)
    {}   

    /**
     *  Construct a basic return frame from a received frame
     * 
     *  @param  received frame
     */ 
    BasicReturnFrame(ReceivedFrame &frame) :
        BasicFrame(frame),
        _replyCode(frame.nextInt16()),
        _replyText(frame),
        _exchange(frame),
        _routingKey(frame)
    {}

    /**
     *  Destructor
     */
    virtual ~BasicReturnFrame() {}

    /**
     *  Is this a synchronous frame?
     *
     *  After a synchronous frame no more frames may be
     *  sent until the accompanying -ok frame arrives
     */
    virtual bool synchronous() const override
    {
        return false;
    }

    /**
     *  Return the name of the exchange to publish to
     *  @return  string
     */
    const std::string& exchange() const
    {
        return _exchange;
    }

    /**
     *  Return the routing key
     *  @return  string
     */
    const std::string& routingKey() const
    {
        return _routingKey;
    }

    /**
     *  Return the method ID
     *  @return  uint16_t
     */
    virtual uint16_t methodID() const override
    {
        return 50;
    }

    /**
     *  Return the reply text
     *  @return  string
     */
    const std::string& replyText() const
    {
        return _replyText;
    }

    /**
     *  Return the reply code
     *  @return  int16_t
     */
    int16_t replyCode() const
    {
        return _replyCode;
    }

    /**
     *  Process the frame
     *  @param  connection      The connection over which it was received
     *  @return bool            Was it succesfully processed?
     */
    virtual bool process(ConnectionImpl *connection) override
    {
        unused(connection);
        // we no longer support returned messages
        return false;
    }
};

/**
 *  end namespace
 */
}

