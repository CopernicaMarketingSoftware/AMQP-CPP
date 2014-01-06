/**
 *  Available field types for AMQP
 * 
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Base field class
 *
 *  This class cannot be constructed, but serves
 *  as the base class for all AMQP field types
 */
class Field
{
protected:
    /**
     *  Decode a field by fetching a type and full field from a frame
     *  The returned field is allocated on the heap!
     *  @param  frame
     *  @return Field*
     */
    static Field *decode(ReceivedFrame &frame);
    
public:
    /**
     *  Destructor
     */
    virtual ~Field() {}

    /**
     *  Create a new instance on the heap of this object, identical to the object passed
     *  @return Field*
     */
    virtual Field *clone() const = 0;

    /**
     *  Get the size this field will take when
     *  encoded in the AMQP wire-frame format
     *  @return size_t
     */
    virtual size_t size() const = 0;

    /**
     *  Write encoded payload to the given buffer.
     *  @param  buffer
     */
    virtual void fill(OutBuffer& buffer) const = 0;

    /**
     *  Get the type ID that is used to identify this type of
     *  field in a field table
     *  @return char
     */
    virtual char typeID() const = 0;
    
    
};

/**
 *  end namespace
 */
}

