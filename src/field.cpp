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
 *  @return std::unique_ptr<Field>
 */
std::unique_ptr<Field> Field::decode(InBuffer &frame)
{
    // get the type
    uint8_t type = frame.nextUint8();
    
    // create field based on type
    switch (type)
    {
        // @todo: use std::make_unique when switching to C++14/17/20
        case 't':   return std::unique_ptr<Field>(new BooleanSet(frame));
        case 'b':   return std::unique_ptr<Field>(new Octet(frame));
        case 'B':   return std::unique_ptr<Field>(new UOctet(frame));
        case 'U':   return std::unique_ptr<Field>(new Short(frame));
        case 'u':   return std::unique_ptr<Field>(new UShort(frame));
        case 'I':   return std::unique_ptr<Field>(new Long(frame));
        case 'i':   return std::unique_ptr<Field>(new ULong(frame));
        case 'L':   return std::unique_ptr<Field>(new LongLong(frame));
        case 'l':   return std::unique_ptr<Field>(new ULongLong(frame));
        case 'f':   return std::unique_ptr<Field>(new Float(frame));
        case 'd':   return std::unique_ptr<Field>(new Double(frame));
        case 'D':   return std::unique_ptr<Field>(new DecimalField(frame));
        case 's':   return std::unique_ptr<Field>(new ShortString(frame));
        case 'S':   return std::unique_ptr<Field>(new LongString(frame));
        case 'A':   return std::unique_ptr<Field>(new Array(frame));
        case 'T':   return std::unique_ptr<Field>(new Timestamp(frame));
        case 'F':   return std::unique_ptr<Field>(new Table(frame));
        case 'V':   return std::unique_ptr<Field>(new VoidField(frame));
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

