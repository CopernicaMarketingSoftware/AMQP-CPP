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
     *  First set of booleans
     *  @var    BooleanSet
     */
    BooleanSet _bools1;
    
    /**
     *  Second set of booleans
     *  @var    BooleanSet
     */
    BooleanSet _bools2;
    
    /**
     *  MIME content type
     *  @var    ShortString
     */
    ShortString _contentType;

    /**
     *  MIME content encoding
     *  @var    ShortString
     */
    ShortString _contentEncoding;

    /**
     *  message header field table
     *  @var    Table
     */
    Table _headers;

    /**
     *  Delivery mode (non-persistent (1) or persistent (2))
     *  @var    uint8_t
     */
    uint8_t _deliveryMode;

    /**
     *  boolean whether field was sent to us
     *  @var    uint8_t
     */
    uint8_t _priority;

    /**
     *  application correlation identifier
     *  @var    ShortString
     */
    ShortString _correlationID;

    /**
     *  address to reply to
     *  @var    ShortString
     */
    ShortString _replyTo;

    /**
     *  message expiration identifier
     *  @var    ShortString
     */
    ShortString _expiration;

    /**
     *  application message identifier
     *  @var    ShortString
     */
    ShortString _messageID;

    /**
     *  message timestamp
     *  @var    Timestamp
     */
    Timestamp _timestamp;

    /**
     *  message type name
     *  @var    ShortString
     */
    ShortString _typeName;

    /**
     *  creating user id
     *  @var    ShortString
     */
    ShortString _userID;

    /**
     *  creating application id
     *  @var    ShortString
     */
    ShortString _appID;

    /**
     *  Deprecated cluster ID
     *  @var    ShortString
     */
    ShortString _clusterID;

    

protected:
    /**
     *  Encode a header frame to a string buffer
     *
     *  @param  buffer  buffer to write frame to
     */
    virtual void fill(OutBuffer& buffer) const override
    {
        // call base
        HeaderFrame::fill(buffer);

        // fill own fields.
        buffer.add(_weight);
        buffer.add(_bodySize);
        _bools1.fill(buffer);
        _bools2.fill(buffer);

        if (contentTypeSent() )     { _contentType.fill(buffer); }
        if (contentEncodingSent() ) { _contentEncoding.fill(buffer); }
        if (headersSent() )         { _headers.fill(buffer); }
        if (deliveryModeSent() )    { buffer.add(_deliveryMode); }
        if (prioritySent() )        { buffer.add(_priority); }
        if (correlationIDSent() )   { _correlationID.fill(buffer); }
        if (replyToSent() )         { _replyTo.fill(buffer); }
        if (expirationSent() )      { _expiration.fill(buffer); }
        if (messageIDSent() )       { _messageID.fill(buffer); }
        if (timestampSent() )       { _timestamp.fill(buffer); }
        if (typeNameSent() )        { _typeName.fill(buffer); }
        if (userIDSent() )          { _userID.fill(buffer); }
        if (appIDSent() )           { _appID.fill(buffer); }
    }

public:
    /**
     *  Construct an empty basic header frame
     *
     *  All options are set using setter functions.
     * 
     *  @param  channel     channel we're working on
     */
    BasicHeaderFrame(uint16_t channel, uint64_t bodySize) : 
        HeaderFrame(channel, 12), // there are at least 12 bytes sent, weight (2), bodySize (8), property flags (2)
        _weight(0),
        _bodySize(bodySize),
        _deliveryMode(0),
        _priority(0)
    {}

    /**
     *  Constructor to parse incoming frame
     *  @param  frame
     */
    BasicHeaderFrame(ReceivedFrame &frame) : 
        HeaderFrame(frame),
        _weight(frame.nextUint16()),
        _bodySize(frame.nextUint64()),
        _bools1(frame),
        _bools2(frame),
        _deliveryMode(0),
        _priority(0)
    {
        if (contentTypeSent()) _contentType = ShortString(frame);
        if (contentEncodingSent()) _contentEncoding = ShortString(frame);
        if (headersSent()) _headers = Table(frame);
        if (deliveryModeSent()) _deliveryMode = frame.nextUint8();
        if (prioritySent()) _priority = frame.nextUint8();
        if (correlationIDSent()) _correlationID = ShortString(frame);
        if (replyToSent()) _replyTo = ShortString(frame);
        if (expirationSent()) _expiration = ShortString(frame);
        if (messageIDSent()) _messageID = ShortString(frame);
        if (timestampSent()) _timestamp = Timestamp(frame);
        if (typeNameSent()) _typeName = ShortString(frame);
        if (userIDSent()) _userID = ShortString(frame);
        if (appIDSent()) _appID = ShortString(frame);
        if (clusterIDSent()) _clusterID = ShortString(frame);
    }

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

    /**
     *  Set the body size
     *  @param  uint64_t    sum of all body-sizes sent after this headerframe
     */
    void setBodySize(uint64_t size)
    {
        _bodySize = size;
    }

    /**
     *  return the MIME content type
     *  @return string
     */
    const std::string& contentType() const
    {
        return _contentType;
    }
    
    /**
     *  Set the content type 
     *  @param  string
     */
    void setContentType(std::string& string)
    {
        // was there already a content type
        if (contentTypeSent()) modifySize(-_contentType.size());

        // set the new content type
        setContentTypeSent(string.size() > 0);
        _contentType = ShortString(string);

        // modify the size to include the new content type
        if (contentTypeSent()) modifySize(_contentType.size());
    }
    
    /**
     *  Set the bool for content type sent
     *  @param bool
     */
    void setContentTypeSent(bool b)
    {
        _bools1.set(7, b);
    }

    /**
     *  return the MIME content encoding
     *  @return string
     */
    const std::string& contentEncoding() const
    {
        return _contentEncoding;
    }
    
    /**
     *  Set content encoding
     *  @param string
     */
    void setContentEncoding(std::string& string)
    {
        // was there already a content encoding?
        if(contentEncodingSent()) modifySize(-_contentEncoding.size());

        // set new content encoding
        setContentEncodingSent(string.size() > 0);
        _contentEncoding = ShortString(string);

        // modify size to include the new content type
        modifySize(_contentEncoding.size());
    }
    
    /**
     *  set contentencoding sent
     *  @param bool
     */
    void setContentEncodingSent(bool b)
    {
        _bools1.set(6, b);
    }
    
    /**
     *  return the message header field table
     *  @return Table
     */
    const Table& headers() const
    {
        return _headers;
    }
    
    /**
     *  Set headers
     *  @param Table
     */
    void setHeaders(Table& t)
    {
        // were the headers already set
        if(headersSent()) modifySize(-_headers.size());

        // set new headers
        setHeadersSent(true);
        _headers = t;

        // modify size to include the new headers
        modifySize(_headers.size());
    }
    
    /**
     *  Set headers sent
     *  @param bool
     */
    void setHeadersSent(bool b)
    {
        _bools1.set(5, b);
    }

    /**
     *  return whether non-persistent (1) or persistent (2)
     *  @return uint8_t
     */
    uint8_t deliveryMode() const
    {
        return _deliveryMode;
    }
    
    /**
     *  Set deliverymode
     *  @param uint8_t
     */
    void setDeliveryMode(uint8_t val)
    {
        // was the delivery mode already set
        if(deliveryModeSent()) modifySize(-1);

        // set delivery mode
        setDeliverModeSent(true);
        _deliveryMode = Octet(val);

        // add new size
        modifySize(1);
    }
    
    /**
     *  set delivermode sent
     *  @param bool
     */
    void setDeliverModeSent(bool b)
    {
        _bools1.set(4, b);
    }
    
    /**
     *  return the message priority (0-9)
     *  @return uint8_t
     */
    uint8_t priority() const
    {
        return _priority;
    }
    
    /**
     *  Set priority
     *  @param uint8_t
     */
    void setPriority(uint8_t val)
    {
        // was the priority already sent
        if(prioritySent()) modifySize(-1);

        // set priority
        setPrioritySent(true);
        _priority = Octet(val);

        // add new size
        modifySize(1);
    }

    /**
     *  Set priority sent
     *  @param  bool
     */
    void setPrioritySent(bool b)
    {
        _bools1.set(3, b);
    }

    /**
     *  return the application correlation identifier
     *  @return string
     */
    const std::string& correlationID() const
    {
        return _correlationID;
    }
    
    /**
     *  set correlation ID
     *  @param string
     */
    void setCorrelationID(std::string &s)
    {
        // was the correlation ID sent
        if(correlationIDSent()) modifySize(-_correlationID.size());

        // set new correlation ID
        setCorrelationIDSent(true);
        _correlationID = ShortString(s);

        // add new size
        modifySize(_correlationID.size());
    }
    
    /**
     *  Set correlationIDSent
     *  @param bool
     */
    void setCorrelationIDSent(bool b)
    {
        _bools1.set(2, b);
    }

    /**
     *  return the address to reply to
     *  @return string
     */
    const std::string& replyTo() const
    {
        return _replyTo;
    }
    
    /**
     *  Set reply to
     *  @param string
     */
    void setReplyTo(std::string &s)
    {
        // was replyTo set?
        if(replyToSent()) modifySize(-_replyTo.size());

        // add new replyTo
        setReplyToSent(true);
        _replyTo = ShortString(s);

        modifySize(_replyTo.size());
    }
    
    /**
     *  set reply to sent
     *  @param bool
     */
    void setReplyToSent(bool b)
    {
        _bools1.set(1, b);
    }

    /**
     *  return the message expiration identifier
     *  @return string
     */
    const std::string& expiration() const
    {
        return _expiration;
    }
    
    /**
     *  Set expiration
     *  @param string
     */
    void setExpiration(std::string &s)
    {
        // was expiration set?
        if(expirationSent()) modifySize(-_expiration.size());

        // set expiration
        setExpirationSent(true);
        _expiration = ShortString(s);

        // add new size
        modifySize(_expiration.size());
    }
    
    /**
     *  set expiration sent
     *  @param bool
     */
    void setExpirationSent(bool b)
    {
        _bools1.set(0, b);
    }

    /**
     *  return the application message identifier
     *  @return string
     */
    const std::string& messageID() const
    {
        return _messageID;
    }

    /**
     *  set message ID
     *  @param string
     */
    void setMessageID(std::string &s)
    {
        // was message ID sent?
        if(messageIDSent()) modifySize(-_messageID.size());

        // set messageID
        setMessageIDSent(true);
        _messageID = ShortString(s);

        // add size
        modifySize(_messageID.size());
    }

    /**
     *  set messageID sent
     *  @param bool
     */
    void setMessageIDSent(bool b)
    {
        _bools2.set(7, b);
    }

    /**
     *  return the message timestamp
     *  @return uint64_t
     */
    Timestamp timestamp() const
    {
        return _timestamp;
    }
    
    /**
     *  set timestamp
     *  @param uint64_t
     */
    void setTimestamp(uint64_t val)
    {
        // was timestamp sent?
        if(timestampSent()) modifySize(-_timestamp.size());

        // set timestamp
        setTimestampSent(true);
        _timestamp = Timestamp(val);

        // add new size
        modifySize(_timestamp.size());
    }
    
    /**
     *  set timestamp sent
     *  @param bool
     */
    void setTimestampSent(bool b)
    {
        _bools2.set(6, b);
    }

    /**
     *  return the message type name
     *  @return string
     */
    const std::string& typeName() const
    {
        return _typeName;
    }
    
    /**
     *  set typename
     *  @param string
     */
    void setTypeName(std::string &s)
    {
        // was typename sent?
        if(typeNameSent()) modifySize(-_typeName.size());

        // add typename
        setTypeNameSent(true);
        _typeName = ShortString(s);

        // add new size
        modifySize(_typeName.size());
    }
    
    /**
     *  set typename sent
     *  @param bool
     */
    void setTypeNameSent(bool b)
    {
        _bools2.set(5, b);
    }

    /**
     *  return the creating user id
     *  @return string
     */
    const std::string& userID() const
    {
        return _userID;
    }
    
    /**
     *  set User ID
     *  @param string
     */
    void setUserID(std::string &s)
    {
        // was user id sent?
        if(userIDSent()) modifySize(-_userID.size());

        // set new userID
        setUserIDSent(true);
        _userID = ShortString(s);

        // add size
        modifySize(_userID.size());
    }
    
    /**
     *  set user id sent
     *  @param bool
     */
    void setUserIDSent(bool b)
    {
        _bools2.set(4, b);
    }

    /**
     *  return the  application id
     *  @return string
     */
    const std::string& appID() const
    {
        return _appID;
    }
    
    /**
     *  set appID
     *  @param string
     */
    void setAppID(std::string &s)
    {
        // was app id sent?
        if(appIDSent()) modifySize(-_appID.size());

        // add new app id
        setAppIDSent(true);
        _appID = ShortString(s);

        // add size
        modifySize(_appID.size());
    }
    
    /**
     *  set app id sent
     *  @param bool
     */
    void setAppIDSent(bool b)
    {
        _bools2.set(3, b);
    }

    /**
     *  Return whether a field was sent
     *  @return bool
     */
    bool expirationSent() const         { return _bools1.get(0); }
    bool replyToSent() const            { return _bools1.get(1); }
    bool correlationIDSent() const      { return _bools1.get(2); }
    bool prioritySent() const           { return _bools1.get(3); }
    bool deliveryModeSent() const       { return _bools1.get(4); }
    bool headersSent() const            { return _bools1.get(5); }
    bool contentEncodingSent() const    { return _bools1.get(6); }
    bool contentTypeSent() const        { return _bools1.get(7); }
    bool clusterIDSent() const          { return _bools2.get(2); }
    bool appIDSent() const              { return _bools2.get(3); }
    bool userIDSent() const             { return _bools2.get(4); }
    bool typeNameSent() const           { return _bools2.get(5); }
    bool timestampSent() const          { return _bools2.get(6); }
    bool messageIDSent() const          { return _bools2.get(7); }

};

/**
 *  End namespace
 */
}

