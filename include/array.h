#pragma once
/**
 *  AMQP field array
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  AMQP field array
 */
class Array : public Field
{
private:
    /**
     *  Definition of an array as a vector
     *  @typedef
     */
    typedef std::vector<std::shared_ptr<Field>> FieldArray;

    /**
     *  The actual fields
     *  @var FieldArray
     */
    FieldArray _fields;

public:
    /**
     *  Constructor to construct an array from a received frame
     *
     *  @param  frame   received frame
     */
    Array(ReceivedFrame &frame);

    /**
     *  Copy constructor
     *  @param  array
     */
    Array(const Array &array);

    /**
     *  Move constructor
     *  @param  array
     */
    Array(Array &&array) : _fields(std::move(array._fields)) {}

    /**
     *  Constructor for an empty Array
     */
    Array() {}

    /**
     * Destructor
     */
    virtual ~Array() {}

    /**
     *  Create a new instance of this object
     *  @return Field*
     */
    virtual Field *clone() const override
    {
        return new Array(*this);
    }

    /**
     *  Get the size this field will take when
     *  encoded in the AMQP wire-frame format
     *  @return size_t
     */
    virtual size_t size() const override;

    /**
     *  Set a field
     *
     *  @param  index   field index
     *  @param  value   field value
     *  @return Array
     */
    Array set(uint8_t index, const Field &value)
    {
        // copy to a new pointer and store it
        _fields[index] = std::shared_ptr<Field>(value.clone());

        // allow chaining
        return *this;
    }

    /**
     *  Get a field
     *
     *  If the field does not exist, an empty string is returned
     *
     *  @param  index   field index
     *  @return Field
     */
    const Field &get(uint8_t index) const;

    /**
     * Get a string item
     * @param index index of item into array
     * @return the string or an empty one if it's not a string
     */
    std::string getString(uint8_t index) const;

    /**
     * Get a table item
     * @param index index of item into array
     * @return the table or an empty one if it's not a table
     */
    AMQP::Table getTable(uint8_t index) const;

    /**
     *  Get number of elements on this array
     *
     *  @return array size
     */
    uint32_t count() const;

    /**
     * Remove last element from array
     */
    void pop_back();

    /**
     * Add field to end of array
     *
     * @param value
     */
    void push_back(const Field &value);

    /**
     *  Get a field
     *
     *  @param  index   field index
     *  @return ArrayFieldProxy
     */
    ArrayFieldProxy operator[](uint8_t index)
    {
        return ArrayFieldProxy(this, index);
    }

    Array& operator=(const Array& a)
    {
        _fields = a._fields;
        return *this;
    }

    /**
     *  Write encoded payload to the given buffer.
     *  @param  buffer
     */
    virtual void fill(OutBuffer& buffer) const override;

    /**
     *  Get the type ID that is used to identify this type of
     *  field in a field table
     *  @return char
     */
    virtual char typeID() const override
    {
        return 'A';
    }
};

/**
 *  end namespace
 */
}

