/**
 *  Field.cpp
 *
 *  @copyright 2014 - 2020 Copernica BV
 */
#include "includes.h"

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Decode a field by fetching a type and full field from a frame
 *  The returned field is allocated on the heap!
 *  @param  frame
 *  @return std::shared_ptr<Field>
 */
std::shared_ptr<Field> Field::decode(InBuffer &frame)
{
    // get the type
    uint8_t type = frame.nextUint8();
    
    // create field based on type
    switch (type)
    {
        case 't':   return std::make_shared<BooleanSet>(frame);
        case 'b':   return std::make_shared<Octet>(frame);
        case 'B':   return std::make_shared<UOctet>(frame);
        case 'U':   return std::make_shared<Short>(frame);
        case 'u':   return std::make_shared<UShort>(frame);
        case 'I':   return std::make_shared<Long>(frame);
        case 'i':   return std::make_shared<ULong>(frame);
        case 'L':   return std::make_shared<LongLong>(frame);
        case 'l':   return std::make_shared<ULongLong>(frame);
        case 'f':   return std::make_shared<Float>(frame);
        case 'd':   return std::make_shared<Double>(frame);
        case 'D':   return std::make_shared<DecimalField>(frame);
        case 's':   return std::make_shared<ShortString>(frame);
        case 'S':   return std::make_shared<LongString>(frame);
        case 'A':   return std::make_shared<Array>(frame);
        case 'T':   return std::make_shared<Timestamp>(frame);
        case 'F':   return std::make_shared<Table>(frame);
        case 'V':   return std::make_shared<VoidField>(frame);
        default:    return nullptr;
    }
}

/**
 *  Cast to string
 *  @return std::string
 */
Field::operator const std::string& () const
{
    // static empty string
    static std::string empty;
    
    // return it
    return empty;
}

/**
 *  Cast to array
 *  @return Array
 */
Field::operator const Array& () const
{
    // static empty array
    static Array empty;
    
    // return it
    return empty;
}

/**
 *  Cast to object
 *  @return Array
 */
Field::operator const Table& () const
{
    // static empty table
    static Table empty;
    
    // return it
    return empty;
}

/**
 *  End of namespace
 */
}

