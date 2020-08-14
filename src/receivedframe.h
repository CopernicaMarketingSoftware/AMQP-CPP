/**
 *  ReceivedFrame.h
 *
 *  The received frame class is a wrapper around a data buffer, it tries to
 *  find out if the buffer is big enough to contain an entire frame, and
 *  it will try to recognize the frame type in the buffer
 *
 *  This is a class that is used internally by the AMQP library. As a user
 *  of this library, you normally do not have to instantiate it.
 *
 *  @copyright 2014 - 2020 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <cstdint>

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Forward declarations
 */
class ConnectionImpl;

/**
 *  Class definition
 */
class ReceivedFrame : public InBuffer
{
private:
    /**
     *  Type of frame
     *  @var    uint8_t
     */
    uint8_t _type = 0;

    /**
     *  Channel identifier
     *  @var    uint16_t
     */
    uint16_t _channel = 0;

    /**
     *  The payload size
     *  @var    uint32_t
     */
    uint32_t _payloadSize = 0;


    /**
     *  Process a method frame
     *  @param  connection
     *  @return bool
     */
    bool processMethodFrame(ConnectionImpl *connection);

    /**
     *  Process a connection frame
     *  @param  connection
     *  @return bool
     */
    bool processConnectionFrame(ConnectionImpl *connection);

    /**
     *  Process a channel frame
     *  @param  connection
     *  @return bool
     */
    bool processChannelFrame(ConnectionImpl *connection);

    /**
     *  Process an exchange frame
     *  @param  connection
     *  @return bool
     */
    bool processExchangeFrame(ConnectionImpl *connection);

    /**
     *  Process a queue frame
     *  @param  connection
     *  @return bool
     */
    bool processQueueFrame(ConnectionImpl *connection);

    /**
     *  Process a basic frame
     *  @param  connection
     *  @return bool
     */
    bool processBasicFrame(ConnectionImpl *connection);

    /**
     *  Process a confirm frame
     *  @param  connection
     *  @return bool
     */
    bool processConfirmFrame(ConnectionImpl *connection);

    /**
     *  Process a transaction frame
     *  @param  connection
     *  @return bool
     */
    bool processTransactionFrame(ConnectionImpl *connection);

    /**
     *  Process a header frame
     *  @param  connection
     *  @return bool
     */
    bool processHeaderFrame(ConnectionImpl *connection);


public:
    /**
     *  Constructor
     *  @param  buffer      Binary buffer
     *  @param  max         Max buffer size
     */
    ReceivedFrame(const Buffer &buffer, uint32_t max);

    /**
     *  Destructor
     */
    virtual ~ReceivedFrame() {}

    /**
     *  Have we at least received the full frame header?
     *  The header contains the frame type, the channel ID and the payload size
     *  @return bool
     */
    bool header() const;

    /**
     *  Is this a complete frame?
     *  @return bool
     */
    bool complete() const;

    /**
     *  Return the channel identifier
     *  @return uint16_t
     */
    uint16_t channel() const
    {
        return _channel;
    }

    /**
     *  Total size of the frame (headers + payload)
     *  @return uint32_t
     */
    uint64_t totalSize() const
    {
        // payload size + size of headers and end of frame byte
        return _payloadSize + 8;
    }

    /**
     *  The size of the payload
     *  @return uint32_t
     */
    uint32_t payloadSize() const
    {
        return _payloadSize;
    }

    /**
     *  Process the received frame
     *
     *  If this method returns false, it means that the frame was not processed,
     *  because it was an unrecognized frame. This does not mean that the
     *  connection is now in an invalid state however.
     *
     *  @param  connection  the connection over which the data was received
     *  @return bool        was the frame fully processed
     *  @internal
     */
    bool process(ConnectionImpl *connection);
};

/**
 *  End of namespace
 */
}
