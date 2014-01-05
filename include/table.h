/**
 *  AMQP field table
 * 
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  AMQP field table
 */
class Table : public Field
{
private:
    /**
     *  We define a custom type for storing fields
     *  @typedef    FieldMap
     */
    typedef std::map<std::string, std::shared_ptr<Field> > FieldMap;

    /**
     *  Store the fields
     *  @var    FieldMap
     */
    FieldMap _fields;

public:
    /**
     *  Constructor that creates an empty table
     */
    Table() {}

    /**
     *  Decode the data from a received frame into a table
     *
     *  @param  frame   received frame to decode
     */
    Table(ReceivedFrame &frame);
    
    /**
     *  Copy constructor
     *  @param  table
     */
    Table(const Table &table);

    /**
     *  Move constructor
     *  @param  table
     */
    Table(Table &&table) : _fields(std::move(table._fields)) {}

    /**
     *  Destructor
     */
    virtual ~Table() {}

    /**
     *  Assignment operator
     *  @param  table
     *  @return Table
     */
    Table &operator=(const Table &table);
    
    /**
     *  Move assignment operator
     *  @param  table
     *  @return Table
     */
    Table &operator=(Table &&table);

    /**
     *  Create a new instance on the heap of this object, identical to the object passed
     *  @return Field*
     */
    virtual Field *clone() const override
    {
        return new Table(*this);
    }

    /**
     *  Get the size this field will take when
     *  encoded in the AMQP wire-frame format
     */
    virtual size_t size() const override;

    /**
     *  Set a field
     *
     *  @param  name    field name
     *  @param  value   field value
     */
    Table set(const std::string& name, const Field &value)
    {
        // copy to a new pointer and store it
        _fields[name] = std::shared_ptr<Field>(value.clone());

        // allow chaining
        return *this;
    }

    /**
     *  Get a field
     * 
     *  If the field does not exist, an empty string field is returned
     *
     *  @param  name    field name
     *  @return         the field value
     */
    const Field &get(const std::string &name);

    /**
     *  Get a field
     *
     *  @param  name    field name
     */
    AssociativeFieldProxy operator[](const std::string& name)
    {
        return AssociativeFieldProxy(this, name);
    }

    /**
     *  Write encoded payload to the given buffer. 
     *  @param  buffer
     */
    virtual void fill(OutBuffer& buffer) const override;

    /**
     *  Get the type ID that is used to identify this type of
     *  field in a field table
     */
    virtual char typeID() const override
    {
        return 'F';
    }
};

/**
 *  end namespace
 */
}

