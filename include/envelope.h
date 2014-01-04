/**
 *  Envelope.h
 *
 *  When you send or receive a message to the rabbitMQ server, it is encapsulated
 *  in an envelope that contains additional meta information as well.
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
class Envelope
{
private:
    /**
     *  Pointer to the body data (the memory buffer is not managed by the AMQP
     *  library!)
     *  @var    const char *
     */
    const char *_data;
    
    /**
     *  Size of the data
     *  @var    size_t
     */
    size_t _size;
    
    /**
     *  The content type
     *  @var    EnvelopeField
     */
    EnvelopeField<std::string> _contentType;
    
    /**
     *  The content encoding
     *  @var    EnvelopeField
     */
    EnvelopeField<std::string> _contentEncoding;

    /**
     *  The priority
     *  @var    EnvelopeField
     */
    EnvelopeField<uint8_t> _priority;
    
    /**
     *  The delivery mode (1=non-persistent, 2=persistent)
     *  @var    EnvelopeField
     */
    EnvelopeField<uint8_t> _deliveryMode;
    
    /**
     *  The correlation ID
     *  @var    EnvelopeField
     */
    EnvelopeField<std::string> _correlationID;
    
    /**
     *  Reply-to field
     *  @var    EnvelopeField
     */
    EnvelopeField<std::string> _replyTo;
    
    /**
     *  Expiration value
     *  @var    EnvelopeField
     */
    EnvelopeField<std::string> _expiration;
    
    /**
     *  The message id
     *  @var    EnvelopeField
     */
    EnvelopeField<std::string> _messageID;
    
    /**
     *  Timestamp
     *  @var    EnvelopeField
     */
    EnvelopeField<uint64_t> _timestamp;
    
    /**
     *  The type name
     *  @var    EnvelopeField
     */
    EnvelopeField<std::string> _typeName;
    
    /**
     *  The user ID
     *  @var    EnvelopeField
     */
    EnvelopeField<std::string> _userID;
    
    /**
     *  The application ID
     *  @var    EnvelopeField
     */
    EnvelopeField<std::string> _appID;
    
    /**
     *  Additional custom headers
     *  @var    EnvelopeField
     */
    EnvelopeField<Table> _headers;
    

public:
    /**
     *  Constructor
     * 
     *  The data buffer that you pass to this constructor must be valid during
     *  the lifetime of the Envelope object.
     * 
     *  @param  data
     *  @param  size
     */
    Envelope(const char *data, size_t size) : _data(data), _size(size) {}
    
    /**
     *  Constructor based on a string
     *  @param  message
     */
    Envelope(const std::string &data) : _data(data.data()), _size(data.size()) {}

    /**
     *  Destructor
     */
    virtual ~Envelope() {}
    
    /**
     *  Access to the full message data
     *  @return buffer
     */
    const char *body()
    {
        return _data;
    }
    
    /**
     *  Size of the body
     *  @return size_t
     */
    size_t size()
    {
        return _size;
    }
    
    /**
     *  Check if a certain field is set
     *  @return bool
     */
    bool hasPriority        () { return _priority.valid();          }
    bool hasDeliveryMode    () { return _deliveryMode.valid();      }
    bool hasTimestamp       () { return _timestamp.valid();         }
    bool hasContentType     () { return _contentType.valid();       }
    bool hasContentEncoding () { return _contentEncoding.valid();   }
    bool hasCorrelationID   () { return _correlationID.valid();     }
    bool hasReplyTo         () { return _replyTo.valid();           }
    bool hasExpiration      () { return _expiration.valid();        }
    bool hasMessageID       () { return _messageID.valid();         }
    bool hasTypeName        () { return _typeName.valid();          }
    bool hasUserID          () { return _userID.valid();            }
    bool hasAppID           () { return _appID.valid();             }
    bool hasHeaders         () { return _headers.valid();           }
    
    /**
     *  Set the various supported fields
     *  @param  value
     */
    void setPriority        (uint8_t value)            { _priority          = value; }
    void setDeliveryMode    (uint8_t value)            { _deliveryMode      = value; }
    void setTimestamp       (uint64_t value)           { _timestamp         = value; }
    void setContentType     (const std::string &value) { _contentType       = value; }
    void setContentEncoding (const std::string &value) { _contentEncoding   = value; }
    void setCorrelationID   (const std::string &value) { _correlationID     = value; }
    void setReplyTo         (const std::string &value) { _replyTo           = value; }
    void setExpiration      (const std::string &value) { _expiration        = value; }
    void setMessageID       (const std::string &value) { _messageID         = value; }
    void setTypeName        (const std::string &value) { _typeName          = value; }
    void setUserID          (const std::string &value) { _userID            = value; }
    void setAppID           (const std::string &value) { _appID             = value; }
    void setHeaders         (const Table &value)       { _headers           = value; }
    
    /**
     *  Reset the various supported fields
     *  @param  value
     */
    void setPriority        (nullptr_t value = nullptr) { _priority         = value; }
    void setDeliveryMode    (nullptr_t value = nullptr) { _deliveryMode     = value; }
    void setTimestamp       (nullptr_t value = nullptr) { _timestamp        = value; }
    void setContentType     (nullptr_t value = nullptr) { _contentType      = value; }
    void setContentEncoding (nullptr_t value = nullptr) { _contentEncoding  = value; }
    void setCorrelationID   (nullptr_t value = nullptr) { _correlationID    = value; }
    void setReplyTo         (nullptr_t value = nullptr) { _replyTo          = value; }
    void setExpiration      (nullptr_t value = nullptr) { _expiration       = value; }
    void setMessageID       (nullptr_t value = nullptr) { _messageID        = value; }
    void setTypeName        (nullptr_t value = nullptr) { _typeName         = value; }
    void setUserID          (nullptr_t value = nullptr) { _userID           = value; }
    void setAppID           (nullptr_t value = nullptr) { _appID            = value; }
    void setHeaders         (nullptr_t value = nullptr) { _headers          = value; }
    
    /**
     *  Retrieve the fields
     *  @return string
     */
    uint8_t      priority       () { return _priority;          }
    uint8_t      deliveryMode   () { return _deliveryMode;      }
    uint64_t     timestamp      () { return _timestamp;         }
    std::string &contentType    () { return _contentType;       }
    std::string &contentEncoding() { return _contentEncoding;   }
    std::string &correlationID  () { return _correlationID;     }
    std::string &replyTo        () { return _replyTo;           }
    std::string &expiration     () { return _expiration;        }
    std::string &messageID      () { return _messageID;         }
    std::string &typeName       () { return _typeName;          }
    std::string &userID         () { return _userID;            }
    std::string &appID          () { return _appID;             }
    Table       &headers        () { return _headers;           }
    
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
        if (value) setDeliveryMode(2);
        else setDeliveryMode(nullptr);
    }
};

/**
 *  End of namespace
 */
}

