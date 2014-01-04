/**
 *  Class describing an AMQP transaction commit ok frame
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
class TransactionCommitOKFrame : public TransactionFrame
{
protected:
    /**
     *  Encode a frame on a string buffer
     *
     *  @param  buffer  buffer to write frame to
     */
    virtual void fill(OutBuffer& buffer) const override
    {
        // call base
        TransactionFrame::fill(buffer);
    }

public:
    /**
     *  Construct a transaction commit ok frame
     * 
     *  @param   channel     channel identifier
     *  @return  newly created transaction commit ok frame
     */
    TransactionCommitOKFrame(uint16_t channel) :
        TransactionFrame(channel, 0)
    {}

    /**
     *  Constructor on incoming data
     *
     *  @param   frame   received frame to decode
     */
    TransactionCommitOKFrame(ReceivedFrame& frame) :
        TransactionFrame(frame)
    {}

    /**
     *  Destructor
     */
    virtual ~TransactionCommitOKFrame() {}

    /**
     *  return the method id
     *  @return uint16_t
     */
    virtual uint16_t methodID() const override
    {
        return 21;
    }
};

/**
 *  end namespace
 */
}

