#pragma once
/**
 *  Numeric field types for AMQP
 * 
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Template for numeric field types
 */
template<
    typename T,
    char F,
    typename = typename std::enable_if<std::is_arithmetic<T>::value, T>,
    typename = typename std::enable_if<std::is_integral<T>::value, T>
>
class NumericField : public Field
{
private:
    /**
     *  Field value
     */
    T _value;

public:
    /**
     *  Default constructor, assign 0
     */
    NumericField() : _value(0) {}

    /**
     *  Construct numeric field from
     *  one of numeric types
     *
     *  @param  value   field value
     */
    NumericField(T value) : _value(value) {}

    /**
     *  Parse based on incoming buffer
     *  @param  frame
     */
    NumericField(ReceivedFrame &frame)
    {
        // copy the data from the buffer into the field
        if (!std::is_floating_point<T>::value)
        {
            // convert value based on internal storage size
            switch (sizeof(T))
            {
                case 1: _value = frame.nextUint8();  break;
                case 2: _value = frame.nextUint16(); break;
                case 4: _value = frame.nextUint32(); break;
                case 8: _value = frame.nextUint64(); break;
            }
        }
        else
        {
            switch (sizeof(T))
            {
                case 4: _value = frame.nextFloat();  break;
                case 8: _value = frame.nextDouble(); break;
            }
        }
    }

    /**
     *  Destructor
     */
    virtual ~NumericField() {}

    /**
     *  Create a new instance of this object
     *  @return Field*
     */
    virtual std::shared_ptr<Field> clone() const override
    {
        // create a new copy of ourselves and return it
        return std::make_shared<NumericField>(_value);
    }

    /**
     *  Assign a new value
     *
     *  @param  value   new value for field
     *  @return NumericField
     */
    NumericField& operator=(T value)
    {
        _value = value;
        return *this;
    };

    /**
     *  Get the value
     *  @return mixed
     */
    virtual operator T () const override
    {
        return _value;
    }

    /**
     *  Get the value
     *  @return mixed
     */
    T value() const
    {
        // return internal value
        return _value;
    }

    /**
     *  Get the size this field will take when
     *  encoded in the AMQP wire-frame format
     *  @return size_t
     */
    virtual size_t size() const override
    {
        // numeric types have no extra storage requirements
        return sizeof(_value);
    }

    /**
     *  Get the maximum allowed value for this field
     *  @return mixed
     */
    constexpr static T max()
    {
        return std::numeric_limits<T>::max();
    }

    /**
     *  Write encoded payload to the given buffer.
     *  @param  buffer      OutBuffer to write to
     */
    virtual void fill(OutBuffer& buffer) const override
    {
        // store converted value
        T value = _value;

        // write to buffer
        // adding a value takes care of host to network byte order
        buffer.add(value);
    }

    /**
     *  Get the type ID that is used to identify this type of
     *  field in a field table
     */
    virtual char typeID() const override
    {
        return F;
    }

    /**
     *  Output the object to a stream
     *  @param std::ostream
     */
    virtual void output(std::ostream &stream) const override
    {
        // show
        stream << "numeric(" << value() << ")";
    }
};

/**
 *  Concrete numeric types for AMQP
 */
typedef NumericField<int8_t, 'b'>   Octet;
typedef NumericField<uint8_t, 'B'>  UOctet;
typedef NumericField<int16_t, 'U'>  Short;
typedef NumericField<uint16_t, 'u'> UShort;
typedef NumericField<int32_t, 'I'>  Long;
typedef NumericField<uint32_t, 'i'> ULong;
typedef NumericField<int64_t, 'L'>  LongLong;
typedef NumericField<uint64_t, 'l'> ULongLong;
typedef NumericField<uint64_t, 'T'> Timestamp;

/**
 *  Concrete floating-point types for AMQP
 */
typedef NumericField<float, 'f'>    Float;
typedef NumericField<double, 'd'>   Double;

/**
 *  end namespace
 */
}

