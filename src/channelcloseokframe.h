/**
 *  Class describing a channel close acknowledgement frame
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
class ChannelCloseOKFrame : public ChannelFrame
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
        ChannelFrame::fill(buffer);
    }

public:
    /**
     *  Construct a channel close ok  frame
     *  @param  frame
     */
    ChannelCloseOKFrame(ReceivedFrame &frame) :
        ChannelFrame(frame)
    {}

    /**
     *  Construct a channel close ok  frame
     *
     *  @param  channel     channel we're working on
     */
    ChannelCloseOKFrame(uint16_t channel) :
        ChannelFrame(channel, 0)
    {}

    /**
     *  Destructor
     */
    virtual ~ChannelCloseOKFrame() {}

    /**
     *  Method id
     */
    virtual uint16_t methodID() const override
    {
        return 41;
    }

    /**
     *  Process the frame
     *  @param  connection      The connection over which it was received
     *  @return bool            Was it succesfully processed?
     */
    virtual bool process(ConnectionImpl *connection) override;
};

/**
 *  end namespace
 */
}

