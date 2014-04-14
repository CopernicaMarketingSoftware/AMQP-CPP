/**
 *  Base class for a message implementation
 *
 *  This is the base class for either the returned message or the consumed
 *  message.
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class MessageImpl : public Message
{
private:
    /**
     *  How many bytes have been received?
     *  @var uint64_t
     */
    uint64_t _received;
    uint64_t _bodySize;

protected:
    /**
     *  Constructor
     *  @param  exchange
     *  @param  routingKey
     */
    MessageImpl(const std::string &exchange, const std::string &routingKey) :
        Message(exchange, routingKey),
        _received(0),_bodySize(0)
        {}

public:

    /**
     *  Set the body size
     *  This field is set when the header is received
     *  @param  uint64_t
     */
    void setBodySize(uint64_t size)
    {
        _bodySize = size;
    }

    /**
     *  Append data
     *  @param  buffer      incoming data
     *  @param  size        size of the data
     *  @return bool        true if the message is now complete
     */
    bool append(const char *buffer, uint64_t size)
    {
        _str.append(buffer, size);
        _received += size;
        return _received >= _bodySize;
    }

    /**
     *  Report to the handler
     *  @param  channel
     *  @param  handler
     */
    virtual void report(Channel *channel, ChannelHandler *handler) = 0;
};

/**
 *  End of namespace
 */
}

