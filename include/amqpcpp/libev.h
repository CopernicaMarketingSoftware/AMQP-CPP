/**
 *  LibEV.h
 *
 *  Implementation for the AMQP::TcpHandler that is optimized for libev. You can
 *  use this class instead of a AMQP::TcpHandler class, just pass the event loop
 *  to the constructor and you're all set
 *
 *  Compile with: "g++ -std=c++11 libev.cpp -lamqpcpp -lev -lpthread"
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 - 2018 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <ev.h>
#include "amqpcpp/linux_tcp.h"

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class LibEvHandler : public TcpHandler
{
private:
    /**
     *  Internal interface for the object that is being watched
     */
    class Watchable
    {
    public:
        /**
         *  The method that is called when a filedescriptor becomes active
         *  @param  fd
         *  @param  events
         */
        virtual void onActive(int fd, int events) = 0;
        
        /**
         *  Method that is called when the timer expires
         */
        virtual void onExpired() = 0;
    };

    /**
     *  Helper class that wraps a libev I/O watcher
     */
    class Watcher
    {
    private:
        /**
         *  The event loop to which it is attached
         *  @var struct ev_loop
         */
        struct ev_loop *_loop;

        /**
         *  The actual watcher structure
         *  @var struct ev_io
         */
        struct ev_io _io;

        /**
         *  Callback method that is called by libev when a filedescriptor becomes active
         *  @param  loop        The loop in which the event was triggered
         *  @param  w           Internal watcher object
         *  @param  revents     Events triggered
         */
        static void callback(struct ev_loop *loop, struct ev_io *watcher, int revents)
        {
            // retrieve the watched object
            Watchable *object = static_cast<Watchable*>(watcher->data);

            // tell the object that its filedescriptor is active
            object->onActive(watcher->fd, revents);
        }

    public:
        /**
         *  Constructor
         *  @param  loop            The current event loop
         *  @param  object          The object being watched
         *  @param  fd              The filedescriptor being watched
         *  @param  events          The events that should be monitored
         */
        Watcher(struct ev_loop *loop, Watchable *object, int fd, int events) : _loop(loop)
        {
            // initialize the libev structure
            ev_io_init(&_io, callback, fd, events);

            // store the object in the data "void*"
            _io.data = object;

            // start the watcher
            ev_io_start(_loop, &_io);
        }

        /**
         *  Watchers cannot be copied or moved
         *
         *  @param  that    The object to not move or copy
         */
        Watcher(Watcher &&that) = delete;
        Watcher(const Watcher &that) = delete;

        /**
         *  Destructor
         */
        virtual ~Watcher()
        {
            // stop the watcher
            ev_io_stop(_loop, &_io);
        }
        
        /**
         *  Check if a filedescriptor is covered by the watcher
         *  @param  fd
         *  @return bool
         */
        bool contains(int fd) const { return _io.fd == fd; }

        /**
         *  Change the events for which the filedescriptor is monitored
         *  @param  events
         */
        void events(int events)
        {
            // stop the watcher if it was active
            ev_io_stop(_loop, &_io);

            // set the events
            ev_io_set(&_io, _io.fd, events);

            // and restart it
            ev_io_start(_loop, &_io);
        }
    };

    /**
     *  Wrapper around a connection, this will monitor the filedescriptors
     *  and run a timer to send out heartbeats
     */
    class Wrapper : private Watchable
    {
    private:
        /**
         *  The connection that is monitored
         *  @var TcpConnection
         */
        TcpConnection *_connection;
    
        /**
         *  The event loop to which it is attached
         *  @var struct ev_loop
         */
        struct ev_loop *_loop;

        /**
         *  The watcher for the timer
         *  @var struct ev_io
         */
        struct ev_timer _timer;
        
        /**
         *  IO-watchers to monitor filedescriptors
         *  @var std::vector
         */
        std::vector<std::unique_ptr<Watcher>> _watchers;
        
        /**
         *  When should we send the next heartbeat?
         *  @var ev_tstamp
         */
        ev_tstamp _next;
        
        /**
         *  When does the connection expire / was the server for a too longer period of time idle?
         *  @var ev_tstamp
         */
        ev_tstamp _expire;
        
        /**
         *  Interval between heartbeats
         *  @var uint16_t
         */
        uint16_t _interval;
        

        /**
         *  Callback method that is called by libev when the timer expires
         *  @param  loop        The loop in which the event was triggered
         *  @param  timer       Internal timer object
         *  @param  revents     The events that triggered this call
         */
        static void callback(struct ev_loop *loop, struct ev_timer *timer, int revents)
        {
            // retrieve the object
            Watchable *object = static_cast<Watchable*>(timer->data);

            // tell the object that it expired
            object->onExpired();
        }
        
        /**
         *  Method that is called when the timer expired
         */
        virtual void onExpired() override
        {
            // get the current time
            ev_tstamp now = ev_now(_loop);
            
            // should we send out a new heartbeat?
            if (now >= _next)
            {
                // send a heartbeat frame
                _connection->heartbeat();
                
                // remember when we should send out the next one
                _next += _interval;
            }
            
            // was the server idle for a too longer period of time?
            if (now >= _expire)
            {
                // close the connection with immediate effect (this will destruct the connection)
                // @todo do we want to report an error too?
                _connection->close(true);
            }
            else
            {
                // find the earliest thing that expires
                _timer.repeat = std::min(_next, _expire) - now;
                
                // restart the timer
                ev_timer_again(_loop, &_timer);
            }
        }
        
        /**
         *  Method that is called when a filedescriptor becomes active
         *  @param  fd          the filedescriptor that is active
         *  @param  events      the events that are active (readable/writable)
         */
        virtual void onActive(int fd, int events) override
        {
            // if the server is readable, we have some extra time before it expires
            if (events & EV_READ) _expire = ev_now(_loop) + _interval * 2;
            
            // pass on to the connection
            _connection->process(fd, events);
        }
        

    public:
        /**
         *  Constructor
         *  @param  loop            The current event loop
         *  @param  connection      The TCP connection
         *  @param  interval        Timer interval
         */
        Wrapper(struct ev_loop *loop, AMQP::TcpConnection *connection, uint16_t timeout) : 
            _connection(connection),
            _loop(loop),
            _next(ev_now(loop) + timeout),
            _expire(ev_now(loop) + timeout * 2),
            _interval(timeout)
        {
            // store the object in the data "void*"
            _timer.data = this;
            
            // initialize the libev structure
            ev_timer_init(&_timer, callback, timeout, timeout);

            // and start it
            ev_timer_start(_loop, &_timer);

            // the timer should not keep the event loop active
            ev_unref(_loop);
        }

        /**
         *  Watchers cannot be copied or moved
         *
         *  @param  that    The object to not move or copy
         */
        Wrapper(Wrapper &&that) = delete;
        Wrapper(const Wrapper &that) = delete;

        /**
         *  Destructor
         */
        virtual ~Wrapper()
        {
            // restore loop refcount
            ev_ref(_loop);

            // stop the timer
            ev_timer_stop(_loop, &_timer);
        }
        
        /**
         *  Expose the selected heartbeat interval
         *  @return uint16_t
         */
        uint16_t interval() const
        {
            return _interval;
        }
        
        /**
         *  Check if the timer is associated with a certain connection
         *  @param  connection
         *  @return bool
         */
        bool contains(const AMQP::TcpConnection *connection) const
        {
            // compare the connections
            return _connection == connection;
        }
        
        /**
         *  Monitor a filedescriptor
         *  @param  fd
         *  @param  events
         */
        void monitor(int fd, int events)
        {
            // should we remove?
            if (events == 0)
            {
                // remove the io-watcher
                _watchers.erase(std::remove_if(_watchers.begin(), _watchers.end(), [fd](const std::unique_ptr<Watcher> &watcher) -> bool {
                    
                    // compare filedescriptors
                    return watcher->contains(fd);
                }), _watchers.end());
            }
            else
            {
                // look in the array for this filedescriptor
                for (auto &watcher : _watchers)
                {
                    // do we have a match?
                    if (watcher->fd() != fd) continue;
                
                    // change the events (and leap out)
                    return watcher->events(events);
                }
                
                // we should monitor a new filedescriptor
                _watchers.emplace_back(new Watcher(_loop, this, fd, events));
            }
        }
    };

    /**
     *  The event loop
     *  @var struct ev_loop*
     */
    struct ev_loop *_loop;

    /**
     *  Each connection is wrapped
     *  @var std::vector
     */
    std::vector<std::unique_ptr<Wrapper>> _wrappers;


    /**
     *  Lookup a connection-wrapper
     *  @param  connection
     *  @return Wrapper
     */
    Wrapper *lookup(TcpConnection *connection)
    {
        // look for the appropriate connection
        for (auto &wrapper : _wrappers)
        {
            // do we have a match?
            if (wrapper->contains(connection)) return wrapper.get();
        }
        
        // we need a new wrapper
        auto *wrapper = new Wrapper(_loop, connection, 60);

        // add to the wrappers
        _wrappers.emplace_back(wrapper);
        
        // done
        return wrapper;
    }

    /**
     *  Method that is called by AMQP-CPP to register a filedescriptor for readability or writability
     *  @param  connection  The TCP connection object that is reporting
     *  @param  fd          The filedescriptor to be monitored
     *  @param  flags       Should the object be monitored for readability or writability?
     */
    virtual void monitor(TcpConnection *connection, int fd, int flags) override
    {
        // lookup the appropriate wrapper and start monitoring
        lookup(connection)->monitor(fd, flags);
    }

protected:
    /**
     *  Method that is called when the heartbeat frequency is negotiated between the server and the client. 
     *  @param  connection      The connection that suggested a heartbeat interval
     *  @param  interval        The suggested interval from the server
     *  @return uint16_t        The interval to use
     */
    virtual uint16_t onNegotiate(TcpConnection *connection, uint16_t interval) override
    {
        // lookup the wrapper
        return lookup(connection)->interval();
    }

    /**
     *  Method that is called when the TCP connection is destructed
     *  @param  connection  The TCP connection
     */
    virtual void onDetached(TcpConnection *connection) override
    {
        // remove from the array
        _wrappers.erase(std::remove_if(_wrappers.begin(), _wrappers.end(), [connection](const std::unique_ptr<Wrapper> &wrapper) -> bool {
            return wrapper->contains(connection);
        }), _wrappers.end());
    }

public:
    /**
     *  Constructor
     *  @param  loop    The event loop to wrap
     */
    LibEvHandler(struct ev_loop *loop) : _loop(loop) {}

    /**
     *  Destructor
     */
    virtual ~LibEvHandler() = default;
};

/**
 *  End of namespace
 */
}
