/**
 *  DeferredCancel.h
 *
 *  Deferred callback for instructions that cancel a running consumer. This
 *  deferred object allows one to register a callback that also gets the
 *  consumer tag as one of its parameters.
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  We extend from the default deferred and add extra functionality
 */
class DeferredCancel : public Deferred
{
private:
    /**
     *  Callback to execute when the instruction is completed
     *  @var    CancelCallback
     */
    CancelCallback _cancelCallback;

    /**
     *  Report success for frames that report cancel operations
     *  @param  name            Consumer tag that is cancelled
     *  @return Deferred
     */
    virtual Deferred *reportSuccess(const std::string &name) const override
    {
        // skip if no special callback was installed
        if (!_cancelCallback) return Deferred::reportSuccess();
        
        // call the callback
        _cancelCallback(name);
        
        // call finalize callback
        if (_finalizeCallback) _finalizeCallback();
        
        // return next object
        return _next;
    }

    /**
     *  The channel implementation may call our
     *  private members and construct us
     */
    friend class ChannelImpl;
    friend class ConsumedMessage;
    
protected:
    /**
     *  Protected constructor that can only be called
     *  from within the channel implementation
     *
     *  @param  boolean     are we already failed?
     */
    DeferredCancel(bool failed = false) : Deferred(failed) {}

public:
    /**
     *  Register a function to be called when the cancel operation succeeded
     *
     *  Only one callback can be registered. Successive calls
     *  to this function will clear callbacks registered before.
     *
     *  @param  callback    the callback to execute
     */
    DeferredCancel &onSuccess(const CancelCallback &callback)
    {
        // store callback
        _cancelCallback = callback;
        
        // allow chaining
        return *this;
    }

    /**
     *  All the onSuccess() functions defined in the base class are accessible too
     */
    using Deferred::onSuccess;
};

/**
 *  End namespace
 */
}
