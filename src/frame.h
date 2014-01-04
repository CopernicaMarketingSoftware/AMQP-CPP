/**
 *  Frame.h
 * 
 *  Base class for frames. This base class can not be constructed from outside
 *  the library, and is only used internally.
 * 
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace AMQP {
    
/**
 *  Class definition
 */
class Frame
{
protected:
    /**
     *  Protected constructor to ensure that no objects are created from 
     *  outside the library
     */
    Frame() {}
    
public:
    /**
     *  Destructor
     */
    virtual ~Frame() {}

    /**
     *  return the total size of the frame
     *  @return uint32_t
     */
    virtual uint32_t totalSize() const = 0;
    
    /**
     *  Fill an output buffer
     *  @param  buffer
     */
    virtual void fill(OutBuffer &buffer) const = 0;
    
    /**
     *  Is this a frame that is part of the connection setup?
     *  @return bool
     */
    virtual bool partOfHandshake() const
    {
        return false;
    }
    
    /**
     *  Process the frame
     *  @param  connection      The connection over which it was received
     *  @return bool            Was it succesfully processed?
     */
    virtual bool process(ConnectionImpl *connection)
    {
        // no process was implemented
        return false;
    }
};
    
/**
 *  End of namespace
 */
}

