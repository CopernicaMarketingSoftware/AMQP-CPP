#include "includes.h"

// we live in the copernica namespace
namespace AMQP {

/**
 *  Decode the data from a received frame into a table
 *
 *  @param  frame   received frame to decode
 */
Table::Table(ReceivedFrame &frame)
{
    // table buffer begins with the number of bytes to read 
    uint32_t bytesToRead = frame.nextUint32();
    
    // keep going until the correct number of bytes is read. 
    while (bytesToRead > 0)
    {
        // field name and type
        ShortString name(frame);
        
        // subtract number of bytes to read, plus one byte for the decoded type
        bytesToRead -= (name.size() + 1);
        
        // get the field
        Field *field = Field::decode(frame);
        if (!field) continue;
        
        // add field
        _fields[name] = std::shared_ptr<Field>(field);
        
        // subtract size
        bytesToRead -= field->size();
    }
}

/**
 *  Copy constructor
 *  @param  table
 */
Table::Table(const Table &table)
{
    // loop through the table records
    for (auto iter = table._fields.begin(); iter != table._fields.end(); iter++)
    {
        // add the field
        _fields[iter->first] = std::shared_ptr<Field>(iter->second->clone());
    }
}

/**
 *  Assignment operator
 *  @param  table
 *  @return Table
 */
Table &Table::operator=(const Table &table)
{
    // skip self assignment
    if (this == &table) return *this;
    
    // empty current fields
    _fields.clear();
    
    // loop through the table records
    for (auto iter = table._fields.begin(); iter != table._fields.end(); iter++)
    {
        // add the field
        _fields[iter->first] = std::shared_ptr<Field>(iter->second->clone());
    }
    
    // done
    return *this;
}
    
/**
 *  Move assignment operator
 *  @param  table
 *  @return Table
 */
Table &Table::operator=(Table &&table)
{
    // skip self assignment
    if (this == &table) return *this;
    
    // copy fields
    _fields = std::move(table._fields);
    
    // done
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
const Field &Table::get(const std::string &name) const
{
    // we need an empty string
    static ShortString empty;
    
    // locate the element first
    auto iter(_fields.find(name));

    // check whether the field was found
    if (iter == _fields.end()) return empty;

    // done
    return *iter->second;
}

/**
 *  Get the size this field will take when
 *  encoded in the AMQP wire-frame format
 */
size_t Table::size() const
{
    // add the size of the uint32_t indicating the size
    size_t size = 4;

    // iterate over all elements
    for (auto iter(_fields.begin()); iter != _fields.end(); ++iter)
    {
        // get the size of the field name
        ShortString name(iter->first);
        size += name.size();

        // add the size of the field type
        size += sizeof(iter->second->typeID());

        // add size of element to the total
        size += iter->second->size();
    }

    // return the result
    return size;
}

/**
 *  Write encoded payload to the given buffer. 
 */
void Table::fill(OutBuffer& buffer) const
{
    // add size
    buffer.add(static_cast<uint32_t>(size()-4));

    // loop through the fields
    for (auto iter(_fields.begin()); iter != _fields.end(); ++iter)
    {
        // encode the field name
        ShortString name(iter->first);
        name.fill(buffer);
        
        // encode the element type
        buffer.add((uint8_t) iter->second->typeID());

        // encode element
        iter->second->fill(buffer);
    }
}

// end namespace
}
