/**
 *  DeferredPublish.h
 *
 *  Deferred callback for RabbitMQ-specific publisher confirms mechanism per-message.
 *
 *  @author Michael van der Werve <michael.vanderwerve@mailerq.com>
 *  @copyright 2020 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  We extend from the default deferred and add extra functionality
 */
class DeferredPublish : public Deferred
{
private:
    /**
     *  Callback to execute when server confirms that message is processed
     *  @var    AckCallback
     */
    PublishAckCallback _ackCallback;

    /**
     * Callback to execute when server sends negative acknowledgement
     * @var     NackCallback
     */
    PublishNackCallback _nackCallback;

    /**
     *  Callback to execute when message is lost (nack / error)
     *  @var    LostCallback
     */
    PublishLostCallback _lostCallback;

    /**
     *  Report an ack, calls the callback.
     */
    void reportAck() 
    {
        // check if the callback is set 
        if (_ackCallback) _ackCallback(); 
    }

    /**
     *  Report an nack, calls the callback if set.
     */
    void reportNack() 
    {
        // check if the callback is set 
        if (_nackCallback) _nackCallback(); 

        // message is 'lost'
        if (_lostCallback) _lostCallback();
    }

    /**
     *  Indicate failure
     *  @param  error           Description of the error that occured
     */
    void reportError(const char *error)
    {
        // from this moment on the object should be listed as failed
        _failed = true;

        // message is lost
        if (_lostCallback) _lostCallback();

        // execute callbacks if registered
        if (_errorCallback) _errorCallback(error);
    }

    /**
     *  The wrapped confirmed channel implementation may call our
     *  private members and construct us
     */
    template <class T>
    friend class Reliable;


public:
    /**
     *  Protected constructor that can only be called
     *  from within the channel implementation
     *
     *  Note: this constructor _should_ be protected, but because make_shared
     *  will then not work, we have decided to make it public after all,
     *  because the work-around would result in not-so-easy-to-read code.
     *
     *  @param  boolean     are we already failed?
     */
    DeferredPublish(bool failed = false) : Deferred(failed) {}

public:
    /**
     *  Callback that is called when the broker confirmed message publication
     *  @param  callback    the callback to execute
     */
    inline DeferredPublish &onAck(const PublishAckCallback& callback) { return onAck(PublishAckCallback(callback)); }
    DeferredPublish &onAck(PublishAckCallback&& callback)
    {
        // store callback
        _ackCallback = std::move(callback);

        // allow chaining
        return *this;
    }

    /**
     *  Callback that is called when the broker denied message publication
     *  @param  callback    the callback to execute
     */
    inline DeferredPublish &onNack(const PublishNackCallback& callback) { return onNack(PublishNackCallback(callback)); }
    DeferredPublish &onNack(PublishNackCallback&& callback)
    {
        // store callback
        _nackCallback = std::move(callback);

        // allow chaining
        return *this;
    }

    /**
     *  Callback that is called when a message is lost, either through RabbitMQ
     *  rejecting it or because of a channel error
     *  @param  callback    the callback to execute
     */
    inline DeferredPublish &onLost(const PublishLostCallback& callback) { return onLost(PublishLostCallback(callback)); }
    DeferredPublish &onLost(PublishLostCallback&& callback)
    {
        // store callback
        _lostCallback = std::move(callback);

        // allow chaining
        return *this;
    }
};

/**
 *  End namespace
 */
}
