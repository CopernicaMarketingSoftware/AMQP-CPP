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

/**
 *  Class definition
 */
template <typename... Arguments>
class Deferred
{
private:
    /**
     *  The channel we operate under
     */
    Channel *_channel;

    /**
     *  Do we already know we failed?
     */
    bool _failed;

    /**
     *  Callback to execute on success
     */
    std::function<void(Channel *channel, Arguments ...parameters)> _successCallback;

    /**
     *  Callback to execute on failure
     */
    std::function<void(Channel *channel, const std::string& error)> _errorCallback;

    /**
     *  Callback to execute either way
     */
    std::function<void(Channel *channel, const std::string& error)> _finalizeCallback;

    /**
     *  The channel implementation may call our
     *  private members and construct us
     */
    friend class ChannelImpl;

    /**
     *  Indicate success
     *
     *  @param  parameters...   the extra parameters relevant for this deferred handler
     */
    void success(Arguments ...parameters)
    {
        // execute callbacks if registered
        if (_successCallback)   _successCallback(_channel, parameters...);
        if (_finalizeCallback)  _finalizeCallback(_channel, "");
    }

    /**
     *  Indicate failure
     *
     *  @param  error   description of the error that occured
     */
    void error(const std::string& error)
    {
        // we are now in a failed state
        _failed = true;

        // execute callbacks if registered
        if (_errorCallback)     _errorCallback(_channel, error);
        if (_finalizeCallback)  _finalizeCallback(_channel, error);
    }

    /**
     *  Private constructor that can only be called
     *  from within the channel implementation
     *
     *  @param  channel the channel we operate under
     *  @param  boolea  are we already failed?
     */
    Deferred(Channel *channel, bool failed = false) :
        _channel(channel),
        _failed(failed)
    {}
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
    Deferred& onSuccess(const std::function<void(Channel *channel, Arguments ...parameters)>& callback)
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
    Deferred& onError(const std::function<void(Channel *channel, const std::string& error)>& callback)
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
    Deferred& onFinalize(const std::function<void(Channel *channel, const std::string& error)>& callback)
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
