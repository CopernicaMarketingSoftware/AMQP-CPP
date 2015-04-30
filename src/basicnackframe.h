/**
 *  Class describing a basic negative-acknowledgement frame
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class defintion
 */
class BasicNackFrame : public BasicFrame {
private:
    /**
     *  server-assigned and channel specific delivery tag
     *  @var    uint64_t
     */
    uint64_t _deliveryTag;

    /**
     *  The additional bits
     *  @var    BooleanSet
     */
    BooleanSet _bits;

protected:
    /**
     *  Encode a frame on a string buffer
     *
     *  @param   buffer  buffer to write frame to
     *  @return  pointer to object to allow for chaining
     */
    virtual void fill(OutBuffer& buffer) const override
    {
        // call base
        BasicFrame::fill(buffer);

        // add the delivery tag
        buffer.add(_deliveryTag);

        // add the booleans
        _bits.fill(buffer);
    }

public:
    /**
     *  Construct a basic negative-acknowledgement frame
     *
     *  @param  channel         Channel identifier
     *  @param  deliveryTag     server-assigned and channel specific delivery tag
     *  @param  multiple        nack mutiple messages
     *  @param  requeue         requeue the message
     */
    BasicNackFrame(uint16_t channel, uint64_t deliveryTag, bool multiple = false, bool requeue = false) :
        BasicFrame(channel, 9),
        _deliveryTag(deliveryTag),
        _bits(multiple, requeue) {}

    /**
     *  Construct based on received frame
     *  @param  frame
     */
    BasicNackFrame(ReceivedFrame &frame) :
        BasicFrame(frame),
        _deliveryTag(frame.nextUint64()),
        _bits(frame) {}

    /**
     *  Destructor
     */
    virtual ~BasicNackFrame() {}

    /**
     *  Return the method ID
     *  @return  uint16_t
     */
    virtual uint16_t methodID() const override
    {
        return 120;
    }

    /**
     *  Return the server-assigned and channel specific delivery tag
     *  @return  uint64_t
     */
    uint64_t deliveryTag() const
    {
        return _deliveryTag;
    }

    /**
     *  Return whether to acknowledgement multiple messages
     *  @return  bool
     */
    bool multiple() const
    {
        return _bits.get(0);
    }

    /**
     *  Should the message be put back in the queue?
     *  @return  bool
     */
    bool requeue() const
    {
        return _bits.get(1);
    }
};

/**
 *  end namespace
 */
}

