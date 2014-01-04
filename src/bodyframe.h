/**
 *  Class describing an AMQP Body Frame
 * 
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class implementation
 */ 
class BodyFrame : public ExtFrame
{
private:
    /**
     *  Payload of the frame
     *  Payload can be any number of octets
     *  @var vector<uint8_t>
     */
    std::string _payload;

protected:
    /**
     *  Encode a body frame to a string buffer
     *
     *  @param  buffer  buffer to write frame to
     */
    virtual void fill(OutBuffer& buffer) const override
    {
        // call base
        ExtFrame::fill(buffer);

        // add payload to buffer
        buffer.add(_payload);
    }

public:
    /**
     *  Construct a body frame
     *
     *  @param  channel     channel identifier
     *  @param  payload     payload of the body
     */
    BodyFrame(uint16_t channel, const std::string &payload) :
        ExtFrame(channel, payload.size()),
        _payload(payload)
    {}

    /**
     *  Constructor for incoming data
     *
     *  @param  frame   received frame to decode
     *  @return shared pointer to newly created frame
     */
    BodyFrame(ReceivedFrame& frame) : 
        ExtFrame(frame), 
        _payload(frame.nextData(frame.payloadSize()), frame.payloadSize()) 
    {}

    /**
     *  Destructor
     */
    virtual ~BodyFrame() {}

    /**
     *  Return the type of frame
     *  @return     uint8_t
     */ 
    uint8_t type() const
    {
        return 3;
    }

    /**
     *  Return the payload of the body
     *  @return     vector<UOctet>
     */
    const std::string& payload() const
    {
        return _payload;
    }
};

/**
 *  end namespace
 */
}

