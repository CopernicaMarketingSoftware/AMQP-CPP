/**
 *  Class describing an AMQP transaction rollback ok frame
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
class TransactionRollbackOKFrame : public TransactionFrame
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
     *  Decode a transaction rollback ok frame from a received frame
     *
     *  @param   frame   received frame to decode
     */
    TransactionRollbackOKFrame(ReceivedFrame& frame) :
        TransactionFrame(frame)
    {}

    /**
     *  Construct a transaction rollback ok frame
     * 
     *  @param   channel     channel identifier
     *  @return  newly created transaction rollback ok frame
     */
    TransactionRollbackOKFrame(uint16_t channel) :
        TransactionFrame(channel, 0)
    {}

    /**
     *  Destructor
     */
    virtual ~TransactionRollbackOKFrame() {}

    /**
     * return the method id
     * @return uint16_t
     */
    virtual uint16_t methodID() const override
    {
        return 31;
    }    
};

/**
 *  end namespace
 */
}

