/**
 *  Class describing an AMQP basic header frame
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
class BasicHeaderFrame : public HeaderFrame
{
private:
     /**
     *  Weight field, unused but must be sent, always value 0;
     *  @var uint16_t
     */
    uint16_t _weight;

    /**
     *  Body size, sum of the sizes of all body frames following the content header
     *  @var uint64_t
     */
    uint64_t _bodySize;

    /**
     *  The meta data
     *  @var MetaData
     */
    MetaData _metadata;

protected:
    /**
     *  Encode a header frame to a string buffer
     *
     *  @param  buffer  buffer to write frame to
     */
    virtual void fill(OutBuffer &buffer) const override
    {
        // call base
        HeaderFrame::fill(buffer);

        // fill own fields.
        buffer.add(_weight);
        buffer.add(_bodySize);

        // the meta data
        _metadata.fill(buffer);
    }

public:
    /**
     *  Construct an empty basic header frame
     *
     *  All options are set using setter functions.
     * 
     *  @param  channel     channel we're working on
     *  @param  envelope    the envelope
     */
    BasicHeaderFrame(uint16_t channel, const Envelope &envelope) :
        HeaderFrame(channel, 10 + envelope.size()), // there are at least 10 bytes sent, weight (2), bodySize (8), plus the size of the meta data
        _weight(0),
        _bodySize(envelope.bodySize()),
        _metadata(envelope)
    {}

    /**
     *  Constructor to parse incoming frame
     *  @param  frame
     */
    BasicHeaderFrame(ReceivedFrame &frame) : 
        HeaderFrame(frame),
        _weight(frame.nextUint16()),
        _bodySize(frame.nextUint64()),
        _metadata(frame)
    {}
    
    /**
     *  Destructor
     */
    virtual ~BasicHeaderFrame() {}

    /**
     *  Size of the body
     *  @return uint64_t
     */
    uint64_t bodySize() const
    {
        return _bodySize;
    }

    /**
     *  The class ID
     *  @return uint16_t
     */
    virtual uint16_t classID() const override
    {
        return 60;
    }
};

/**
 *  End namespace
 */
}

