/**
 *  ConnectionImpl.cpp
 *
 *  Implementation of an AMQP connection
 *
 *  @copyright 2014 Copernica BV
 */
#include "includes.h"
#include "protocolheaderframe.h"
#include "exception.h"
#include "protocolexception.h"

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
    // we need a protocol header
    ProtocolHeaderFrame header;
    
    // send out the protocol header
    send(header);
}

/**
 *  Destructor
 */
ConnectionImpl::~ConnectionImpl()
{
    // still connected
    if (_state == state_invalid) return;

    // still in a connected state - should we send the close frame?
    close();
}

/**
 *  Add a channel to the connection, and return the channel ID that it
 *  is allowed to use, or 0 when no more ID's are available
 *  @param  channel
 *  @return uint16_t
 */
uint16_t ConnectionImpl::add(ChannelImpl *channel)
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
void ConnectionImpl::remove(ChannelImpl *channel)
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
 *  @param  size        size of the buffer to decode
 *  @return             number of bytes that were processed
 */
size_t ConnectionImpl::parse(char *buffer, size_t size)
{
    // number of bytes processed
    size_t processed = 0;
    
    // create a monitor object that checks if the connection still exists
    Monitor monitor(this);
    
    // keep looping until we have processed all bytes, and the monitor still
    // indicates that the connection is in a valid state
    while (size > 0 && monitor.valid())
    {
        // prevent protocol exceptions
        try
        {
            // try to recognize the frame
            ReceivedFrame receivedFrame(buffer, size, _maxFrame);
            if (!receivedFrame.complete()) return processed;

            // process the frame
            receivedFrame.process(this);

            // number of bytes processed
            size_t bytes = receivedFrame.totalSize();
            
            // add bytes
            processed += bytes; size -= bytes; buffer += bytes;
        }
        catch (const ProtocolException &exception)
        {
            // something terrible happened on the protocol (like data out of range)
            reportError(exception.what());
            
            // done
            return processed;
        }
    }
    
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
    // leap out if not yet connected
    if (_state != state_connected) return false;
    
    // loop over all channels
    for (auto iter = _channels.begin(); iter != _channels.end(); iter++)
    {
        // close the channel
        iter->second->close();
    }
    
    // we're in a new state
    _state = state_invalid;
    
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
    // so we must monitor if the queue object is still valid after calling the
    // handler
    Monitor monitor(this);
    
    // inform handler
    _handler->onConnected(_parent);
    
    // leap out if the connection no longer exists
    if (!monitor.valid()) return;
    
    // empty the queue of messages
    while (!_queue.empty())
    {
        // get the next message
        OutBuffer buffer(std::move(_queue.front()));

        // remove it from the queue
        _queue.pop();
        
        // send it
        _handler->onData(_parent, buffer.data(), buffer.size());
        
        // leap out if the connection was destructed
        if (!monitor.valid()) return;
    }
}

/**
 *  Send a frame over the connection
 *  @param  frame           The frame to send
 *  @return size_t          Number of bytes sent
 */
size_t ConnectionImpl::send(const Frame &frame)
{
    // we need an output buffer
    OutBuffer buffer(frame.totalSize());
    
    // fill the buffer
    frame.fill(buffer);
    
    // append an end of frame byte (but not when still negotiating the protocol)
    if (frame.needsSeparator()) buffer.add((uint8_t)206);
    
    // are we still setting up the connection?
    if ((_state == state_protocol || _state == state_handshake) && !frame.partOfHandshake())
    {
        // the connection is still being set up, so we need to delay the message sending
        _queue.push(std::move(buffer));
    }
    else
    {
        // send the buffer
        _handler->onData(_parent, buffer.data(), buffer.size());
    }
    
    // done
    return buffer.size();
}

/**
 *  End of namspace
 */
}

