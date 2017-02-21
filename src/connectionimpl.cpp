/**
 *  ConnectionImpl.cpp
 *
 *  Implementation of an AMQP connection
 *
 *  @copyright 2014 - 2016 Copernica BV
 */
#include "includes.h"
#include "protocolheaderframe.h"
#include "connectioncloseokframe.h"
#include "connectioncloseframe.h"
#include "reducedbuffer.h"

/**
 *  set namespace
 */
namespace AMQP {

/**
 *  Construct an AMQP object based on full login data
 *
 *  The first parameter is a handler object. This handler class is
 *  an interface that should be implemented by the caller.
 *
 *  Note that the constructor is private to ensure that nobody can construct
 *  this class, only the real Connection class via a friend construct
 *
 *  @param  parent          Parent connection object
 *  @param  handler         Connection handler
 *  @param  login           Login data
 */
ConnectionImpl::ConnectionImpl(Connection *parent, ConnectionHandler *handler, const Login &login, const std::string &vhost) :
    _parent(parent), _handler(handler), _login(login), _vhost(vhost)
{
    // we need to send a protocol header
    send(ProtocolHeaderFrame());
}

/**
 *  Destructor
 */
ConnectionImpl::~ConnectionImpl()
{
    // close the connection in a nice fashion
    close();

    // invalidate all channels, so they will no longer call methods on this channel object
    for (auto iter = _channels.begin(); iter != _channels.end(); iter++) iter->second->detach();
}

/**
 *  Add a channel to the connection, and return the channel ID that it
 *  is allowed to use, or 0 when no more ID's are available
 *  @param  channel
 *  @return uint16_t
 */
uint16_t ConnectionImpl::add(const std::shared_ptr<ChannelImpl> &channel)
{
    // check if we have exceeded the limit already
    if (_maxChannels > 0 && _channels.size() >= _maxChannels) return 0;

    // keep looping to find an id that is not in use
    while (true)
    {
        // is this id in use?
        if (_nextFreeChannel > 0 && _channels.find(_nextFreeChannel) == _channels.end()) break;

        // id is in use, move on
        _nextFreeChannel++;
    }

    // we have a new channel
    _channels[_nextFreeChannel] = channel;

    // done
    return _nextFreeChannel++;
}

/**
 *  Remove a channel
 *  @param  channel
 */
void ConnectionImpl::remove(const ChannelImpl *channel)
{
    // skip zero channel
    if (channel->id() == 0) return;

    // remove it
    _channels.erase(channel->id());
}

/**
 *  Parse the buffer into a recognized frame
 *
 *  Every time that data comes in on the connection, you should call this method to parse
 *  the incoming data, and let it handle by the AMQP library. This method returns the number
 *  of bytes that were processed.
 *
 *  If not all bytes could be processed because it only contained a partial frame, you should
 *  call this same method later on when more data is available. The AMQP library does not do
 *  any buffering, so it is up to the caller to ensure that the old data is also passed in that
 *  later call.
 *
 *  @param  buffer      buffer to decode
 *  @return             number of bytes that were processed
 */
size_t ConnectionImpl::parse(const Buffer &buffer)
{
    // do not parse if already in an error state
    if (_state == state_closed) return 0;

    // number of bytes processed
    size_t processed = 0;

    // create a monitor object that checks if the connection still exists
    Monitor monitor(this);

    // keep looping until we have processed all bytes, and the monitor still
    // indicates that the connection is in a valid state
    while (processed < buffer.size() && monitor.valid())
    {
        // prevent protocol exceptions
        try
        {
            // try to recognize the frame
            ReducedBuffer reduced_buf(buffer, processed);
            ReceivedFrame receivedFrame(reduced_buf, _maxFrame);
            
            // do we have the full frame?
            if (receivedFrame.complete())
            {
                // process the frame
                receivedFrame.process(this);

                // number of bytes processed
                size_t bytes = receivedFrame.totalSize();

                // add bytes
                processed += bytes;
            }
            else
            {
                // we do not yet have the complete frame, but if we do at least 
                // have the initial bytes of the header, we already know how much 
                // data we need for the next frame, otherwise we need at least 7
                // bytes for processing the header of the next frame
                numeric_cast(_expected, receivedFrame.header() ? receivedFrame.totalSize() : 7);

                // we're ready for now
                return processed;
            }
        }
        catch (const ProtocolException &exception)
        {
            // something terrible happened on the protocol (like data out of range)
            reportError(exception.what());

            // done
            return processed;
        }
    }

    // leap out if the connection object no longer exists
    if (!monitor.valid()) return processed;

    // the entire buffer has been processed, the next call to parse() should at least
    // contain the size of the frame header to be meaningful for the amqp-cpp library
    _expected = 7;

    // if the connection is being closed, we have to do more stuff, otherwise we're ready now
    if (!_closed || _state != state_connected) return processed;

    // the close() function was called, but if the close frame was not yet sent
    // if there are no waiting channels, we can do that right now
    if (!waitingChannels()) sendClose();

    // done
    return processed;
}

/**
 *  Close the connection
 *  This will close all channels
 *  @return bool
 */
bool ConnectionImpl::close()
{
    // leap out if already closed or closing
    if (_closed) return false;

    // mark that the object is closed
    _closed = true;

    // after the send operation the object could be dead
    Monitor monitor(this);

    // number of channels that are waiting for an answer and that have further data
    int waiters = 0;

    // loop over all channels, and close them
    for (auto iter = _channels.begin(); iter != _channels.end(); iter++)
    {
        // close the channel
        iter->second->close();

        // we could be dead now
        if (!monitor.valid()) return true;

        // is this channel waiting for an answer?
        if (iter->second->waiting()) waiters++;
    }

    // if still busy with handshake, we delay closing for a while
    if (waiters > 0 || _state != state_connected) return true;

    // perform the close frame
    sendClose();

    // done
    return true;
}

/**
 *  Method to send the close frames
 *  Returns true if object still exists
 *  @return bool
 */
bool ConnectionImpl::sendClose()
{
    // after the send operation the object could be dead
    Monitor monitor(this);

    // send the close frame
    send(ConnectionCloseFrame(0, "shutdown"));

    // leap out if object no longer is alive
    if (!monitor.valid()) return false;

    // we're in a new state
    _state = state_closing;

    // done
    return true;
}

/**
 *  Mark the connection as connected
 */
void ConnectionImpl::setConnected()
{
    // store connected state
    _state = state_connected;

    // we're going to call the handler, which can destruct the connection,
    // so we must monitor if the queue object is still valid after calling
    Monitor monitor(this);

    // inform handler
    _handler->onConnected(_parent);

    // empty the queue of messages
    while (monitor.valid() && !_queue.empty())
    {
        // get the next message
        OutBuffer buffer(std::move(_queue.front()));

        // remove it from the queue
        _queue.pop();

        // send it
        _handler->onData(_parent, buffer.data(), buffer.size());
    }

    // leap out if object is dead
    if (!monitor.valid()) return;

    // if the close method was called before, and no channel is waiting
    // for an answer, we can now safely send out the close frame
    if (_closed && _state == state_connected && !waiting()) sendClose();
}

/**
 *  Is the connection waiting for an answer from an instruction?
 *  @return bool
 */
bool ConnectionImpl::waiting() const
{
    // some states are implicit waiting states
    if (_state == state_protocol) return true;
    if (_state == state_handshake) return true;
    if (_state == state_closing) return true;
    
    // check if there are waiting channels
    return waitingChannels();
}

/**
 *  Is any channel waiting for an answer on a synchronous call?
 *  @return bool
 */
bool ConnectionImpl::waitingChannels() const
{
    // loop through the channels
    for (auto &iter : _channels)
    {
        // is this a waiting channel
        if (iter.second->waiting()) return true;
    }

    // no waiting channel found
    return false;
}

/**
 *  Send a frame over the connection
 *  @param  frame           The frame to send
 *  @return bool            Was the frame succesfully sent
 */
bool ConnectionImpl::send(const Frame &frame)
{
    // its not possible to send anything if closed or closing down
    if (_state == state_closing || _state == state_closed) return false;

    // some frames can be sent _after_ the close() function was called
    if (_closed && !frame.partOfShutdown() && !frame.partOfHandshake()) return false;

    // if the frame is bigger than we allow on the connection
    // it is impossible to send out this frame successfully
    if (frame.totalSize() > _maxFrame) return false;

    // we need an output buffer
    OutBuffer buffer(frame.buffer());

    // are we still setting up the connection?
    if ((_state == state_connected && _queue.empty()) || frame.partOfHandshake())
    {
        // send the buffer
        _handler->onData(_parent, buffer.data(), buffer.size());
    }
    else
    {
        // the connection is still being set up, so we need to delay the message sending
        _queue.push(std::move(buffer));
    }

    // done
    return true;
}

/**
 *  Send buffered data over the connection
 *
 *  @param  buffer      the buffer with data to send
 */
bool ConnectionImpl::send(OutBuffer &&buffer)
{
    // this only works when we are already connected
    if (_state != state_connected) return false;

    // are we waiting for other frames to be sent before us?
    if (_queue.empty())
    {
        // send it directly
        _handler->onData(_parent, buffer.data(), buffer.size());
    }
    else
    {
        // add to the list of waiting buffers
        _queue.push(std::move(buffer));
    }

    // done
    return true;
}

/**
 *  End of namspace
 */
}

