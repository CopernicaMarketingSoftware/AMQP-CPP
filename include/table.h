#pragma once
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
    virtual std::shared_ptr<Field> clone() const override
    {
        return std::make_shared<Table>(*this);
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
        _fields[name] = value.clone();

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
    const Field &get(const std::string &name) const;

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
     *  Get a field
     *
     *  @param  name    field name
     */
    AssociativeFieldProxy operator[](const char *name)
    {
        return AssociativeFieldProxy(this, name);
    }

    /**
     *  Get a const field
     *
     *  @param  name    field name
     */
    const Field &operator[](const std::string& name) const
    {
        return get(name);
    }

    /**
     *  Get a const field
     *
     *  @param  name    field name
     */
    const Field &operator[](const char *name) const
    {
        return get(name);
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

    /**
     *  Output the object to a stream
     *  @param std::ostream
     */
    virtual void output(std::ostream &stream) const
    {
        // prefix
        stream << "table(";
        
        // is this the first iteration
        bool first = true;
        
        // loop through all members
        for (auto &iter : _fields) 
        {
            // split with comma
            if (!first) stream << ",";
            
            // show output
            stream << iter.first << ":" << *iter.second;
            
            // no longer first iter
            first = false;
        }
        
        // postfix
        stream << ")";
    }

    /**
     *  Cast to table
     *  @return Table
     */
    virtual operator const Table& () const override
    {
        // this already is an array, so no cast is necessary
        return *this;
    }

};

/**
 *  end namespace
 */
}

