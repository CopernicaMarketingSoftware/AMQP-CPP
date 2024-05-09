/**
 *  Numeric field types for AMQP
 *
 *  @copyright 2014 - 2023 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <memory>
#include <type_traits>
#include "inbuffer.h"
#include "outbuffer.h"
#include "field.h"
#include <ostream>

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Begin namespace Details
 */
namespace Details {

/**
 *  Helper function that always returns false -- but dependent on a template type T
 *  @return false
 */
template <class T> constexpr bool alwaysFalse() { return false; }

/**
 *  Metafunction for mapping numeric types to methods of InBuffer
 *  @tparam  T  one of the numeric types
 */
template <class T> struct NumberConvert
{
    // depending on the type T, call the right method of the provided `frame`
    static T apply(InBuffer &frame)
    {
        // the default implementation is to generate a compile error
        static_assert(alwaysFalse<T>(), "missing template specialization for this type");
    }
};

/**
 *  Signed variants
 */
template <> struct NumberConvert<int8_t>  { static int8_t   apply(InBuffer &frame) { return frame.nextInt8();  } };
template <> struct NumberConvert<int16_t> { static int16_t  apply(InBuffer &frame) { return frame.nextInt16(); } };
template <> struct NumberConvert<int32_t> { static int32_t  apply(InBuffer &frame) { return frame.nextInt32(); } };
template <> struct NumberConvert<int64_t> { static int64_t  apply(InBuffer &frame) { return frame.nextInt64(); } };

/**
 *  Unsigned variants
 */
template <> struct NumberConvert<uint8_t>  { static uint8_t  apply(InBuffer &frame) { return frame.nextUint8();  } };
template <> struct NumberConvert<uint16_t> { static uint16_t apply(InBuffer &frame) { return frame.nextUint16(); } };
template <> struct NumberConvert<uint32_t> { static uint32_t apply(InBuffer &frame) { return frame.nextUint32(); } };
template <> struct NumberConvert<uint64_t> { static uint64_t apply(InBuffer &frame) { return frame.nextUint64(); } };

/**
 *  Floating-point variants
 */
template <> struct NumberConvert<float>  { static float  apply(InBuffer &frame) { return frame.nextFloat();  } };
template <> struct NumberConvert<double> { static double apply(InBuffer &frame) { return frame.nextDouble(); } };

/**
 *  End of namespace Details
 */
}

/**
 *  Template for numeric field types
 */
template<
    typename T,
    char F,
    typename = typename std::enable_if<std::is_arithmetic<T>::value, T>
>
class NumericField : public Field
{
private:
    /**
     *  Field value
     */
    T _value;

public:
    using Type = T;

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
    NumericField(InBuffer &frame) : _value(Details::NumberConvert<T>::apply(frame)) {}

    /**
     *  Destructor
     */
    virtual ~NumericField() {}

    /**
     *  Create a new instance of this object
     *  @return unique_ptr
     */
    virtual std::unique_ptr<Field> clone() const override
    {
        // create a new copy of ourselves and return it
        return std::unique_ptr<Field>(new NumericField(_value));
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
     *  Clear the field
     *  @return NumericField
     */
    NumericField& clear()
    {
        _value = 0;
        return *this;
    }

    /**
     *  Get the value
     *  @return mixed
     */
    operator uint8_t () const override { return (uint8_t)_value; }
    operator uint16_t() const override { return (uint16_t)_value; }
    operator uint32_t() const override { return (uint32_t)_value; }
    operator uint64_t() const override { return (uint64_t)_value; }
    operator int8_t  () const override { return (int8_t)_value; }
    operator int16_t () const override { return (int16_t)_value; }
    operator int32_t () const override { return (int32_t)_value; }
    operator int64_t () const override { return (int64_t)_value; }
    operator float () const override { return (float)_value; }
    operator double () const override { return (double)_value; }

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
     *  We are an integer field
     *
     *  @return true, because we are an integer
     */
    bool isInteger() const override
    {
        return std::is_integral<T>::value;
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
