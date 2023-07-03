/**
 *  Class describing a connection unblocked frame
 * 
 *  This frame is sent by the server to the client, when all resource alarms
 *  have cleared and the connection is fully unblocked.
 * 
 *  @copyright 2023 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class implementation
 */
class ConnectionUnblockFrame : public ConnectionFrame
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
        ConnectionFrame::fill(buffer);
    }

public:
    /**
     *  Construct a connection unblocked frame from a received frame
     *
     *  @param frame    received frame
     */
    ConnectionUnblockFrame(ReceivedFrame &frame) :
        ConnectionFrame(frame)
    {}

    /**
     *  Construct a connection unblocked frame
     */
    ConnectionUnblockFrame(uint16_t code, std::string reason) :
        ConnectionFrame(0)
    {}

    /**
     *  Destructor
     */
    virtual ~ConnectionUnblockFrame() {}

    /**
     *  Method id
     *  @return uint16_t
     */
    virtual uint16_t methodID() const override
    {
        return 61;
    }
    
    /**
     *  Process the frame
     *  @param  connection
     */
    virtual bool process(ConnectionImpl *connection) override
    {
        // report that it is no longer blocked
        connection->reportUnblocked();
        
        // done
        return true;
    }
};

/**
 *  end namespace
 */
}

