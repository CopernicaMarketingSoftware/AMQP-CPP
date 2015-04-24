#pragma once
/**
 *  MetaData.h
 *
 *  With every published message a set of meta data is passed to. This class
 *  holds all that meta data.
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class MetaData
{
protected:
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
     *  @var    UOctet
     */
    UOctet _deliveryMode = 0;

    /**
     *  boolean whether field was sent to us
     *  @var    UOctet
     */
    UOctet _priority = 0;

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


    /**
     *  Protected constructor to ensure that this class can only be constructed
     *  in a derived class
     */
    MetaData() {}
    
    
public:
    /**
     *  Read incoming frame
     *  @param  frame
     */
    MetaData(ReceivedFrame &frame) :
        _bools1(frame),
        _bools2(frame)
    {
        // only copy the properties that were sent
        if (hasContentType())       _contentType = ShortString(frame);
        if (hasContentEncoding())   _contentEncoding = ShortString(frame);
        if (hasHeaders())           _headers = Table(frame);
        if (hasDeliveryMode())      _deliveryMode = UOctet(frame);
        if (hasPriority())          _priority = UOctet(frame);
        if (hasCorrelationID())     _correlationID = ShortString(frame);
        if (hasReplyTo())           _replyTo = ShortString(frame);
        if (hasExpiration())        _expiration = ShortString(frame);
        if (hasMessageID())         _messageID = ShortString(frame);
        if (hasTimestamp())         _timestamp = Timestamp(frame);
        if (hasTypeName())          _typeName = ShortString(frame);
        if (hasUserID())            _userID = ShortString(frame);
        if (hasAppID())             _appID = ShortString(frame);
        if (hasClusterID())         _clusterID = ShortString(frame);
    }

    /**
     *  Destructor
     */
    virtual ~MetaData() {}

    /**
     *  Set all meta data
     *  @param  data
     */
    void set(const MetaData &data)
    {
        // simply copy all fields
        _bools1 = data._bools1;
        _bools2 = data._bools2;
        _contentType = data._contentType;
        _contentEncoding = data._contentEncoding;
        _headers = data._headers;
        _deliveryMode = data._deliveryMode;
        _priority = data._priority;
        _correlationID = data._correlationID;
        _replyTo = data._replyTo;
        _expiration = data._expiration;
        _messageID = data._messageID;
        _timestamp = data._timestamp;
        _typeName = data._typeName;
        _userID = data._userID;
        _appID = data._appID;
        _clusterID = data._clusterID;
    }

    /**
     *  Check if a certain field is set
     *  @return bool
     */
    bool hasExpiration      () const { return _bools1.get(0); }
    bool hasReplyTo         () const { return _bools1.get(1); }
    bool hasCorrelationID   () const { return _bools1.get(2); }
    bool hasPriority        () const { return _bools1.get(3); }
    bool hasDeliveryMode    () const { return _bools1.get(4); }
    bool hasHeaders         () const { return _bools1.get(5); }
    bool hasContentEncoding () const { return _bools1.get(6); }
    bool hasContentType     () const { return _bools1.get(7); }
    bool hasClusterID       () const { return _bools2.get(2); }
    bool hasAppID           () const { return _bools2.get(3); }
    bool hasUserID          () const { return _bools2.get(4); }
    bool hasTypeName        () const { return _bools2.get(5); }
    bool hasTimestamp       () const { return _bools2.get(6); }
    bool hasMessageID       () const { return _bools2.get(7); }

    /**
     *  Set the various supported fields
     *  @param  value
     */
    void setExpiration      (const std::string &value) { _expiration        = value; _bools1.set(0,true); }
    void setReplyTo         (const std::string &value) { _replyTo           = value; _bools1.set(1,true); }
    void setCorrelationID   (const std::string &value) { _correlationID     = value; _bools1.set(2,true); }
    void setPriority        (uint8_t value)            { _priority          = value; _bools1.set(3,true); }
    void setDeliveryMode    (uint8_t value)            { _deliveryMode      = value; _bools1.set(4,true); }
    void setHeaders         (const Table &value)       { _headers           = value; _bools1.set(5,true); }
    void setContentEncoding (const std::string &value) { _contentEncoding   = value; _bools1.set(6,true); }
    void setContentType     (const std::string &value) { _contentType       = value; _bools1.set(7,true); }
    void setClusterID       (const std::string &value) { _clusterID         = value; _bools2.set(2,true); }
    void setAppID           (const std::string &value) { _appID             = value; _bools2.set(3,true); }
    void setUserID          (const std::string &value) { _userID            = value; _bools2.set(4,true); }
    void setTypeName        (const std::string &value) { _typeName          = value; _bools2.set(5,true); }
    void setTimestamp       (uint64_t value)           { _timestamp         = value; _bools2.set(6,true); }
    void setMessageID       (const std::string &value) { _messageID         = value; _bools2.set(7,true); }

    /**
     *  Retrieve the fields
     *  @return string
     */
    const std::string &expiration     () const { return _expiration;        }
    const std::string &replyTo        () const { return _replyTo;           }
    const std::string &correlationID  () const { return _correlationID;     }
          uint8_t      priority       () const { return _priority;          }
          uint8_t      deliveryMode   () const { return _deliveryMode;      }
    const Table       &headers        () const { return _headers;           }
    const std::string &contentEncoding() const { return _contentEncoding;   }
    const std::string &contentType    () const { return _contentType;       }
    const std::string &clusterID      () const { return _clusterID;         }
    const std::string &appID          () const { return _appID;             }
    const std::string &userID         () const { return _userID;            }
    const std::string &typeName       () const { return _typeName;          }
          uint64_t     timestamp      () const { return _timestamp;         }
    const std::string &messageID      () const { return _messageID;         }
    
    /**
     *  Is this a message with persistent storage
     *  This is an alias for retrieving the delivery mode and checking if it is set to 2
     *  @return bool
     */
    bool persistent()
    {
        return hasDeliveryMode() && deliveryMode() == 2;
    }
    
    /**
     *  Set whether storage should be persistent or not
     *  @param  bool
     */
    void setPersistent(bool value = true)
    {
        if (value) 
        {
            // simply set the delivery mode
            setDeliveryMode(2);
        }
        else 
        {
            // we remove the field from the header
            _deliveryMode = 0;
            _bools1.set(4,false);
        }
    }

    /**
     *  Total size
     *  @return uint32_t
     */
    uint32_t size() const
    {
        // the result (2 for the two boolean sets)
        uint32_t result = 2;
        
        if (hasExpiration())        result += _expiration.size();
        if (hasReplyTo())           result += _replyTo.size();
        if (hasCorrelationID())     result += _correlationID.size();
        if (hasPriority())          result += _priority.size();
        if (hasDeliveryMode())      result += _deliveryMode.size();
        if (hasHeaders())           result += _headers.size();
        if (hasContentEncoding())   result += _contentEncoding.size();
        if (hasContentType())       result += _contentType.size();
        if (hasClusterID())         result += _clusterID.size();
        if (hasAppID())             result += _appID.size();
        if (hasUserID())            result += _userID.size();
        if (hasTypeName())          result += _typeName.size();
        if (hasTimestamp())         result += _timestamp.size();
        if (hasMessageID())         result += _messageID.size();
        
        // done
        return result;
    }
    
    /**
     *  Fill an output buffer
     *  @param  buffer
     */
    void fill(OutBuffer &buffer) const
    {
        // the two boolean sets are always present
        _bools1.fill(buffer);
        _bools2.fill(buffer);
        
        // only copy the properties that were sent
        if (hasContentType())       _contentType.fill(buffer);
        if (hasContentEncoding())   _contentEncoding.fill(buffer);
        if (hasHeaders())           _headers.fill(buffer);
        if (hasDeliveryMode())      _deliveryMode.fill(buffer);
        if (hasPriority())          _priority.fill(buffer);
        if (hasCorrelationID())     _correlationID.fill(buffer);
        if (hasReplyTo())           _replyTo.fill(buffer);
        if (hasExpiration())        _expiration.fill(buffer);
        if (hasMessageID())         _messageID.fill(buffer);
        if (hasTimestamp())         _timestamp.fill(buffer);
        if (hasTypeName())          _typeName.fill(buffer);
        if (hasUserID())            _userID.fill(buffer);
        if (hasAppID())             _appID.fill(buffer);
        if (hasClusterID())         _clusterID.fill(buffer);
    }
};

/**
 *  End of namespace
 */
}

