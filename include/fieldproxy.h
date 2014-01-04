/**
 *  Field proxy. Returned by the table. Can be casted to the
 *  relevant native type (std::string or numeric)
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
template <typename T, typename I>
class FieldProxy
{
private:
    /**
     *  The table or array possibly holding the requested field
     */
    T *_source;

    /**
     *  The key in the table
     */
    I _index;

public:
    /**
     *  Construct the field proxy
     *
     *  @param  table   the table possibly holding the field
     *  @oaram  index   key in table map
     */
    FieldProxy(T *source, I index) :
        _source(source),
        _index(index)
    {}

    /**
     *  Assign a boolean value
     *
     *  @param  value
     */
    FieldProxy& operator=(bool value)
    {
        // assign value and allow chaining
        _source->set(_index, BooleanSet(value));
        return *this;
    }

    /**
     *  Assign a numeric value
     *
     *  @param  value
     *  @return FieldProxy
     */
    FieldProxy& operator=(uint8_t value)
    {
        // assign value and allow chaining
        _source->set(_index, UOctet(value));
        return *this;
    }

    /**
     *  Assign a numeric value
     *
     *  @param  value
     *  @return FieldProxy
     */
    FieldProxy& operator=(int8_t value)
    {
        // assign value and allow chaining
        _source->set(_index, Octet(value));
        return *this;
    }

    /**
     *  Assign a numeric value
     *
     *  @param  value
     *  @return FieldProxy
     */
    FieldProxy& operator=(uint16_t value)
    {
        // assign value and allow chaining
        _source->set(_index, UShort(value));
        return *this;
    }

    /**
     *  Assign a numeric value
     *
     *  @param  value
     *  @return FieldProxy
     */
    FieldProxy& operator=(int16_t value)
    {
        // assign value and allow chaining
        _source->set(_index, Short(value));
        return *this;
    }

    /**
     *  Assign a numeric value
     *
     *  @param  value
     *  @return FieldProxy
     */
    FieldProxy& operator=(uint32_t value)
    {
        // assign value and allow chaining
        _source->set(_index, ULong(value));
        return *this;
    }

    /**
     *  Assign a numeric value
     *
     *  @param  value
     *  @return FieldProxy
     */
    FieldProxy& operator=(int32_t value)
    {
        // assign value and allow chaining
        _source->set(_index, Long(value));
        return *this;
    }

    /**
     *  Assign a numeric value
     *
     *  @param  value
     *  @return FieldProxy
     */
    FieldProxy& operator=(uint64_t value)
    {
        // assign value and allow chaining
        _source->set(_index, ULongLong(value));
        return *this;
    }

    /**
     *  Assign a numeric value
     *
     *  @param  value
     *  @return FieldProxy
     */
    FieldProxy& operator=(int64_t value)
    {
        // assign value and allow chaining
        _source->set(_index, LongLong(value));
        return *this;
    }

    /**
     *  Assign a decimal value
     *
     *  @param  value
     *  @return FieldProxy
     */
    FieldProxy& operator=(const DecimalField value)
    {
        // assign value and allow chaining
        _source->set(_index, DecimalField(value));
        return *this;
    }

    /**
     *  Assign a string value
     *
     *  @param  value
     *  @return FieldProxy
     */
    FieldProxy &operator=(const std::string &value)
    {
        // in theory we should make a distinction between short and long string,
        // but in practive only long strings are accepted
        _source->set(_index, LongString(value));

        // allow chaining
        return *this;
    }

    /**
     *  Assign a string value
     *
     *  @param  value
     *  @return FieldProxy
     */
    FieldProxy &operator=(const char *value)
    {
        // cast to a string
        return operator=(std::string(value));
    }

    /**
     *  Get boolean value
     *  @return BooleanSet
     */
    operator BooleanSet ()
    {
        // the value
        BooleanSet value;

        // retrieve the value
        _source->get(_index, value);

        // return the result
        return value;
    }

    /**
     *  Get a boolean
     *  @return bool
     */
    operator bool ()
    {
        // the value
        BooleanSet value;

        // retrieve the value
        _source->get(_index, value);

        // return the result
        return value.value();
    }

    /**
     *  Get numeric value
     *  @return int8_t
     */
    operator int8_t ()
    {
        // the value
        Octet value;

        // retrieve the value
        _source->get(_index, value);

        // return the result
        return value.value();
    }

    /**
     *  Get numeric value
     *  @return uint8_t
     */
    operator uint8_t ()
    {
        // the value
        UOctet value;

        // retrieve the value
        _source->get(_index, value);

        // return the result
        return value.value();
    }

    /**
     *  Get numeric value
     *  @return int16_t
     */
    operator int16_t ()
    {
        // the value
        Short value;

        // retrieve the value
        _source->get(_index, value);

        // return the result
        return value.value();
    }

    /**
     *  Get numeric value
     *  @return uint16_t
     */
    operator uint16_t ()
    {
        // the value
        UShort value;

        // retrieve the value
        _source->get(_index, value);

        // return the result
        return value.value();
    }

    /**
     *  Get numeric value
     *  @return int32_t
     */
    operator int32_t ()
    {
        // the value
        Long value;

        // retrieve the value
        _source->get(_index, value);

        // return the result
        return value.value();
    }

    /**
     *  Get numeric value
     *  @return uint32_t
     */
    operator uint32_t ()
    {
        // the value
        ULong value;

        // retrieve the value
        _source->get(_index, value);

        // return the result
        return value.value();
    }

    /**
     *  Get numeric value
     *  @return int64_t
     */
    operator int64_t ()
    {
        // the value
        Long value;

        // retrieve the value
        _source->get(_index, value);

        // return the result
        return value.value();
    }

    /**
     *  Get numeric value
     *  @return uint64_t
     */
    operator uint64_t ()
    {
        // the value
        ULong value;

        // retrieve the value
        _source->get(_index, value);

        // return the result
        return value.value();
    }

    /**
     *  Get decimal value
     *  @return DecimalField
     */
    operator DecimalField ()
    {
        // the value
        DecimalField value;

        // retrieve the value
        _source->get(_index, value);

        // return the result
        return value.value();
    }

    /**
     *  Get string value
     *  @return string
     */
    operator std::string ()
    {
        // it has to be either a short or a long string
        ShortString shortValue;
        LongString longValue;

        // try to retrieve the value
        if (_source->get(_index, shortValue)) return shortValue.value();
        if (_source->get(_index, longValue))  return longValue.value();

        // no valid string found
        return std::string("");
    }
};

// define types for array- and table-based field proxy
typedef FieldProxy<Table, std::string> AssociativeFieldProxy;
typedef FieldProxy<Array, uint8_t> ArrayFieldProxy;

/**
 *  end namespace
 */
}

