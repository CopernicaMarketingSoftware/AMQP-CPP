/**
 *  EnvelopeField.h
 *
 *  An envelope field is a field that also keeps track whether it is set
 *  or not. Used internally by the Envelope class.
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
template <typename T>
class EnvelopeField
{
private:
    /**
     *  The actual value
     *  @var T
     */
    T _value;
    
    /**
     *  Is it set or not
     *  @var bool
     */
    bool _isset;

public:
    /**
     *  Empty constructor
     */
    EnvelopeField() : _isset(false) {}

    /**
     *  Constructor
     *  @param  value
     *  @param  isset
     */
    EnvelopeField(const T &value, bool isset = true) : _value(value), _isset(isset) {}
    
    /**
     *  Destructor
     */
    virtual ~EnvelopeField() {}
    
    /**
     *  Assign a new value
     *  @param  value
     *  @return EnvelopeField
     */
    EnvelopeField &operator=(const T &value)
    {
        _value = value;
        _isset = true;
        return *this;
    }
    
    /**
     *  Reset the value
     *  @param  value
     *  @return EnvelopeField
     */
    EnvelopeField &operator=(nullptr_t value)
    {
        _value = T();
        _isset = false;
        return *this;
    }
    
    /**
     *  Cast to the set value
     *  @return T
     */
    operator T& ()
    {
        return _value;
    }
    
    /**
     *  Reset the value to not being set
     */
    void reset()
    {
        _isset = false;
        _value = T();
    }
    
    /**
     *  Is it set?
     *  @return bool
     */
    bool valid()
    {
        return _isset;
    }
};

/**
 *  End of namespace
 */
}

