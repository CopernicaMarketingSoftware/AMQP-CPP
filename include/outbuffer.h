/**
 *  OutBuffer.h
 *
 *  This is a utility class for writing various data types to a binary
 *  string, and converting the values to network byte order
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
class OutBuffer
{
private:
    /**
     *  Pointer to the beginning of the buffer
     *  @var char*
     */
    char *_buffer;
    
    /**
     *  Pointer to the buffer to be filled
     *  @var char*
     */
    char *_current;

    /**
     *  Current size of the buffer
     *  @var size_t
     */
    size_t _size;
    
    /**
     *  The total capacity of the out buffer
     *  @var size_t
     */
    size_t _capacity;
    

public:
    /**
     *  Constructor
     *  @param  capacity
     */
    OutBuffer(uint32_t capacity)
    {
        // initialize members
        _size = 0;
        _capacity = capacity;
        _buffer = _current = new char[capacity];
    }
    
    /**
     *  Copy constructor
     *  @param  that
     */
    OutBuffer(const OutBuffer &that)
    {
        // initialize members
        _size = that._size;
        _capacity = that._capacity;
        _buffer = new char[_capacity];
        _current = _buffer + _size;
        
        // copy memory
        memcpy(_buffer, that._buffer, _size);
    }
    
    /**
     *  Move constructor
     *  @param  that
     */
    OutBuffer(OutBuffer &&that)
    {
        // copy all members
        _size = that._size;
        _capacity = that._capacity;
        _buffer = that._buffer;
        _current = that._current;
        
        // reset the other object
        that._size = 0;
        that._capacity = 0;
        that._buffer = nullptr;
        that._current = nullptr;
    }

    /**
     *  Destructor
     */
    virtual ~OutBuffer() 
    {
        if (_buffer) delete[] _buffer;
    }

    /**
     *  Get access to the internal buffer
     *  @return const char*
     */
    const char *data()
    {
        return _buffer;
    }

    /**
     *  Current size of the output buffer
     *  @return size_t
     */
    size_t size()
    {
        return _size;
    }

    /**
     *  Add a binary buffer to the buffer
     *  @param  string  char* to the string
     *  @param  size    size of string
     */
    void add(const char *string, uint32_t size)
    {
        memcpy(_current, string, size);
        _current += size;
        _size += size;
    }

    /**
     *  Add a binary buffer to the buffer
     *  @param  string  char* to the string
     *  @param  size    size of string
     */
    void add(const std::string &string)
    {
        add(string.c_str(), string.size());
    }

    /**
     *  add a uint8_t to the buffer
     *  @param value    value to add
     */
    void add(uint8_t value)
    {
        memcpy(_current, &value, sizeof(value));
        _current += sizeof(value);
        _size += sizeof(value);
    }

    /**
     *  add a uint16_t to the buffer
     *  @param value    value to add
     */
    void add(uint16_t value)
    {
        uint16_t v = htobe16(value);
        memcpy(_current, &v, sizeof(v));
        _current += sizeof(v);
        _size += sizeof(v);
    }

    /**
     *  add a uint32_t to the buffer
     *  @param value    value to add
     */
    void add(uint32_t value)
    {
        uint32_t v = htobe32(value);
        memcpy(_current, &v, sizeof(v));
        _current += sizeof(v);
        _size += sizeof(v);
    }

    /**
     *  add a uint64_t to the buffer
     *  @param value    value to add
     */
    void add(uint64_t value)
    {
        uint64_t v = htobe64(value);
        memcpy(_current, &v, sizeof(v));
        _current += sizeof(v);
        _size += sizeof(v);
    }

    /**
     *  add a int8_t to the buffer
     *  @param value    value to add
     */
    void add(int8_t value)
    {
        memcpy(_current, &value, sizeof(value));
        _current += sizeof(value);
        _size += sizeof(value);
    }

    /**
     *  add a int16_t to the buffer
     *  @param value    value to add
     */
    void add(int16_t value)
    {
        int16_t v = htobe16(value);
        memcpy(_current, &v, sizeof(v));
        _current += sizeof(v);
        _size += sizeof(v);
    }

    /**
     *  add a int32_t to the buffer
     *  @param value    value to add
     */
    void add(int32_t value)
    {
        int32_t v = htobe32(value);
        memcpy(_current, &v, sizeof(v));
        _current += sizeof(v);
        _size += sizeof(v);
    }

    /**
     *  add a int64_t to the buffer
     *  @param value    value to add
     */
    void add(int64_t value)
    {
        int64_t v = htobe64(value);
        memcpy(_current, &v, sizeof(v));
        _current += sizeof(v);
        _size += sizeof(v);
    }

    /**
     *  add a float to the buffer
     *  @param value    value to add
     */
    void add(float value)
    {
        memcpy(_current, &value, sizeof(value));
        _current += sizeof(value);
        _size += sizeof(value);
    }

    /**
     *  add a double to the buffer
     *  @param value    value to add
     */
    void add(double value)
    {
        memcpy(_current, &value, sizeof(value));
        _current += sizeof(value);
        _size += sizeof(value);
    }
};

/**
 *  End of namespace
 */
}

