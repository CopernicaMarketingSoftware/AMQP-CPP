/**
 *  Deferred.h
 *
 *  Class describing a set of actions that could
 *  possibly happen in the future that can be
 *  caught.
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace AMQP {

// forward declaration
class ChannelImpl;
class Callbacks;

/**
 *  Class definition
 */
template <typename... Arguments>
class Deferred
{
private:
    /**
     *  Callback to execute on success
     */
    std::function<void(Arguments ...parameters)> _successCallback;

    /**
     *  Callback to execute on failure
     */
    std::function<void(const std::string& error)> _errorCallback;

    /**
     *  Callback to execute either way
     */
    std::function<void(const std::string& error)> _finalizeCallback;

    /**
     *  Indicate success
     *
     *  @param  parameters...   the extra parameters relevant for this deferred handler
     */
    void success(Arguments ...parameters) const
    {
        // execute callbacks if registered
        if (_successCallback)   _successCallback(parameters...);
        if (_finalizeCallback)  _finalizeCallback("");
    }

    /**
     *  Indicate failure
     *
     *  @param  error   description of the error that occured
     */
    void error(const std::string& error) const
    {
        // execute callbacks if registered
        if (_errorCallback)     _errorCallback(error);
        if (_finalizeCallback)  _finalizeCallback(error);
    }

    /**
     *  The channel implementation may call our
     *  private members and construct us
     */
    friend class ChannelImpl;
    friend class Callbacks;
    
protected:
    /**
     *  Do we already know we failed?
     *  @var bool
     */
    bool _failed;

    /**
     *  Protected constructor that can only be called
     *  from within the channel implementation
     *
     *  @param  failed  are we already failed?
     */
    Deferred(bool failed = false) : _failed(failed) {}

public:
    /**
     *  Deleted copy constructor
     */
    Deferred(const Deferred& that) = delete;

    /**
     *  Move constructor
     */
    Deferred(Deferred&& that) :
        _successCallback(std::move(that._successCallback)),
        _errorCallback(std::move(that._errorCallback)),
        _finalizeCallback(std::move(that._finalizeCallback))
    {}

    /**
     *  Cast to a boolean
     */
    operator bool ()
    {
        return !_failed;
    }

    /**
     *  Register a function to be called
     *  if and when the operation succesfully
     *  completes.
     *
     *  Only one callback can be registered at a time.
     *  Successive calls to this function will clear
     *  callbacks registered before.
     *
     *  @param  callback    the callback to execute
     */
    Deferred& onSuccess(const std::function<void(Arguments ...parameters)>& callback)
    {
        // store callback
        _successCallback = callback;
        return *this;
    }

    /**
     *  Register a function to be called
     *  if and when the operation fails.
     *
     *  Only one callback can be registered at a time.
     *  Successive calls to this function will clear
     *  callbacks registered before.
     *
     *  @param  callback    the callback to execute
     */
    Deferred& onError(const std::function<void(const std::string& error)>& callback)
    {
        // store callback
        _errorCallback = callback;
        return *this;
    }

    /**
     *  Register a function to be called
     *  if and when the operation completes
     *  or fails. This function will be called
     *  either way.
     *
     *  In the case of success, the provided
     *  error parameter will be an empty string.
     *
     *  Only one callback can be registered at at time.
     *  Successive calls to this function will clear
     *  callbacks registered before.
     *
     *  @param  callback    the callback to execute
     */
    Deferred& onFinalize(const std::function<void(const std::string& error)>& callback)
    {
        // store callback
        _finalizeCallback = callback;
        return *this;
    }
};

/**
 *  End namespace
 */
}
