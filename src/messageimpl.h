/**
 *  Base class for a message implementation
 *
 *  This is the base class for either the returned message or the consumed
 *  message.
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class MessageImpl : public Message
{
private:
    /**
     *  How many bytes have been received?
     *  @var uint64_t
     */
    uint64_t _received;

    /**
     *  Was the buffer allocated by us?
     *  @var bool
     */
    bool _selfAllocated;

protected:
    /**
     *  Constructor
     *  @param  exchange
     *  @param  routingKey
     */
    MessageImpl(const std::string &exchange, const std::string &routingKey) :
        Message(exchange, routingKey),
        _received(0), _selfAllocated(false)
        {}

public:
    /**
     *  Destructor
     */
    virtual ~MessageImpl()
    {
        // clear up memory if it was self allocated
        if (_selfAllocated) delete[] _body;
    }

    /**
     *  Set the body size
     *  This field is set when the header is received
     *  @param  uint64_t
     */
    void setBodySize(uint64_t size)
    {
        _bodySize = size;
    }

    /**
     *  Append data
     *  @param  buffer      incoming data
     *  @param  size        size of the data
     *  @return bool        true if the message is now complete
     */
    bool append(const char *buffer, uint64_t size)
    {
        // is this the only data, and also direct complete?
        if (_received == 0 && size >= _bodySize)
        {
            // we have everything
            _body = buffer;
            _received = _bodySize;

            // done
            return true;
        }
        else
        {
            // it does not yet fit, do we have to allocate?
            if (!_body) _body = new char[_bodySize];
            _selfAllocated = true;

            // prevent that size is too big
            if (size > _bodySize - _received) size = _bodySize - _received;

            // append data
            memcpy((char *)(_body + _received), buffer, size);

            // we have more data now
            _received += size;

            // done
            return _received >= _bodySize;
        }
    }
};

/**
 *  End of namespace
 */
}

