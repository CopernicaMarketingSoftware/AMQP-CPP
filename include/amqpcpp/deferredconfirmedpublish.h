/**
 *  DeferredConfirmedPublish.h
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
class DeferredConfirmedPublish : public Deferred
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
    }

    /**
     *  The wrapped confirmed channel implementation may call our
     *  private members and construct us
     */
    friend class Confirmed;


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
    DeferredConfirmedPublish(bool failed = false) : Deferred(failed) {}

public:
    /**
     *  Callback that is called when the broker confirmed message publication
     *  @param  callback    the callback to execute
     */
    DeferredConfirmedPublish &onAck(const PublishAckCallback &callback)
    {
        // store callback
        _ackCallback = callback;

        // allow chaining
        return *this;
    }

    /**
     *  Callback that is called when the broker denied message publication
     *  @param  callback    the callback to execute
     */
    DeferredConfirmedPublish &onNack(const PublishNackCallback &callback)
    {
        // store callback
        _nackCallback = callback;

        // allow chaining
        return *this;
    }
};

/**
 *  End namespace
 */
}
