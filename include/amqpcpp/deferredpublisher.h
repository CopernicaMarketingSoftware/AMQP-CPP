/**
 *  DeferredPublisher.h
 * 
 *  Class that is returned when channel::publish() is called, and that
 *  can be used to install callback methods that define how returned
 *  messages should be handled.
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2018 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Begin of namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class DeferredPublisher : public DeferredReceiver
{
private:


public:
    /**
     *  Constructor that should only be called from within the channel implementation
     *
     *  Note: this constructor _should_ be protected, but because make_shared
     *  will then not work, we have decided to make it public after all,
     *  because the work-around would result in not-so-easy-to-read code.
     *
     *  @param  channel     the channel implementation
     *  @param  failed      are we already failed?
     */
    DeferredConsumer(ChannelImpl *channel, bool failed = false) :
        DeferredReceiver(failed, channel) {}
        
public:
    /**
     *  Register a function to be called when a full message is returned
     *  @param  callback    the callback to execute
     */
    DeferredConsumer &onReceived(const ReturnCallback &callback)
    {
        // store callback
        _returnCallback = callback;

        // allow chaining
        return *this;
    }

    /**
     *  Alias for onReceived() (see above)
     *  @param  callback    the callback to execute
     */
    DeferredConsumer &onMessage(const ReturnCallback &callback)
    {
        // store callback
        _returnCallback = callback;

        // allow chaining
        return *this;
    }

    /**
     *  Alias for onReceived() (see above)
     *  @param  callback    the callback to execute
     */
    DeferredConsumer &onReturned(const ReturnCallback &callback)
    {
        // store callback
        _returnCallback = callback;

        // allow chaining
        return *this;
    }

    /**
     *  RabbitMQ sends a message in multiple frames to its consumers.
     *  The AMQP-CPP library collects these frames and merges them into a 
     *  single AMQP::Message object that is passed to the callback that
     *  you can set with the onReceived() or onMessage() methods (see above).
     * 
     *  However, you can also write your own algorithm to merge the frames.
     *  In that case you can install callbacks to handle the frames. Every
     *  message is sent in a number of frames:
     * 
     *      - a begin frame that marks the start of the message
     *      - an optional header if the message was sent with an envelope
     *      - zero or more data frames (usually 1, but more for large messages)
     *      - an end frame to mark the end of the message.
     *  
     *  To install handlers for these frames, you can use the onBegin(), 
     *  onHeaders(), onData() and onComplete() methods.
     * 
     *  If you just rely on the onReceived() or onMessage() callbacks, you
     *  do not need any of the methods below this line.
     */

    /**
     *  Register the function that is called when the start frame of a new 
     *  consumed message is received
     *
     *  @param  callback    The callback to invoke
     *  @return Same object for chaining
     */
    DeferredConsumer &onBegin(const BeginCallback &callback)
    {
        // store callback
        _beginCallback = callback;

        // allow chaining
        return *this;
    }

    /**
     *  Register the function that is called when message headers come in
     *
     *  @param  callback    The callback to invoke for message headers
     *  @return Same object for chaining
     */
    DeferredConsumer &onHeaders(const HeaderCallback &callback)
    {
        // store callback
        _headerCallback = callback;

        // allow chaining
        return *this;
    }

    /**
     *  Register the function to be called when a chunk of data comes in
     *
     *  Note that this function may be called zero, one or multiple times
     *  for each incoming message depending on the size of the message data.
     *
     *  If you install this callback you very likely also want to install
     *  the onComplete callback so you know when the last data part was
     *  received.
     *
     *  @param  callback    The callback to invoke for chunks of message data
     *  @return Same object for chaining
     */
    DeferredConsumer &onData(const DataCallback &callback)
    {
        // store callback
        _dataCallback = callback;

        // allow chaining
        return *this;
    }

    /**
     *  Register a funtion to be called when a message was completely received
     *
     *  @param  callback    The callback to invoke
     *  @return Same object for chaining
     */
    DeferredConsumer &onComplete(const CompleteCallback &callback)
    {
        // store callback
        _completeCallback = callback;

        // allow chaining
        return *this;
    }
};
    
/**
 *  End of namespace
 */
}

