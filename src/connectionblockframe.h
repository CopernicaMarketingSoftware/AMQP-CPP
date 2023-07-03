/**
 *  Class describing a connection blocked frame
 * 
 *  This frame is sent by the server to the client, when their connection gets
 *  blocked for the first time due to the broker running low on a resource
 *  (memory or disk). For example, when a RabbitMQ node detects that it is low
 *  on RAM, it sends a notification to all connected publishing clients
 *  supporting this feature. If before the connections are unblocked the node
 *  also starts running low on disk space, another notification will not be sent.
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
class ConnectionBlockFrame : public ConnectionFrame
{
private:
    /**
     *  The reason for blocking
     *  @var ShortString
     */
    ShortString _reason;

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

        // encode the field
        _reason.fill(buffer);
    }

public:
    /**
     *  Construct a connection blocked frame from a received frame
     *
     *  @param frame    received frame
     */
    ConnectionBlockFrame(ReceivedFrame &frame) :
        ConnectionFrame(frame),
        _reason(frame)
    {}

    /**
     *  Construct a connection blocked frame
     *
     *  @param  reason          the reason for blocking
     */
    ConnectionBlockFrame(uint16_t code, std::string reason) :
        ConnectionFrame((uint32_t)(reason.length() + 1)), // 1 for extra string byte
        _reason(std::move(reason))
    {}

    /**
     *  Destructor
     */
    virtual ~ConnectionBlockFrame() {}

    /**
     *  Method id
     *  @return uint16_t
     */
    virtual uint16_t methodID() const override
    {
        return 60;
    }

    /**
     *  Get the reason for blocking
     *  @return string
     */
    const std::string& reason() const
    {
        return _reason;
    }
    
    /**
     *  Process the frame
     *  @param  connection
     */
    virtual bool process(ConnectionImpl *connection) override
    {
        // report that it is blocked
        connection->reportBlocked(this->reason().c_str());
        
        // done
        return true;
    }
};

/**
 *  end namespace
 */
}

