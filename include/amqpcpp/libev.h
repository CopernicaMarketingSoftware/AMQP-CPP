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
#include <list>
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
         *  @var std::list
         */
        std::list<Watcher> _watchers;
        
        /**
         *  When should we send the next heartbeat?
         *  @var ev_tstamp
         */
        ev_tstamp _next = 0.0;
        
        /**
         *  When does the connection expire / was the server for a too longer period of time idle?
         *  During connection setup, this member is used as the connect-timeout.
         *  @var ev_tstamp
         */
        ev_tstamp _expire;
        
        /**
         *  Interval between heartbeats (we should send every interval / 2 a new heartbeat)
         *  Value zero means heartbeats are disabled, or not yet negotiated.
         *  @var uint16_t
         */
        uint16_t _interval = 0;

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
            
            // if the onNegotiate method was not yet called, and no heartbeat interval was negotiated
            if (_interval == 0)
            {
                // there is a theoretical scenario in which the onNegotiate() method
                // was overridden, so that the connection-timeout-timer expires, but 
                // the connection is ready anyway -- in that case we should ignore the timeout
                if (_connection->ready()) return;
                
                // the timer expired because the connection could not be set up in time,
                // close the connection with immediate effect
                _connection->close(true);
            }
            else
            {
                // the connection is alive, and heartbeats are needed, should we send a new one?
                if (now >= _next)
                {
                    // send a heartbeat frame
                    _connection->heartbeat();
                    
                    // remember when we should send out the next one
                    _next += std::max(_interval / 2, 1);
                }
            
                // was the server idle for a too longer period of time?
                if (now >= _expire)
                {
                    // close the connection with immediate effect (this will destruct the connection)
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
        }
        
        /**
         *  Method that is called when a filedescriptor becomes active
         *  @param  fd          the filedescriptor that is active
         *  @param  events      the events that are active (readable/writable)
         */
        virtual void onActive(int fd, int events) override
        {
            // if the server is readable, we have some extra time before it expires
            if (_interval != 0 && (events & EV_READ)) _expire = ev_now(_loop) + _interval;
            
            // pass on to the connection
            _connection->process(fd, events);
        }
        

    public:
        /**
         *  Constructor
         *  @param  loop            The current event loop
         *  @param  connection      The TCP connection
         *  @param  timeout         Connect timeout
         */
        Wrapper(struct ev_loop *loop, AMQP::TcpConnection *connection, uint16_t timeout = 60) : 
            _connection(connection),
            _loop(loop),
            _next(0.0),
            _expire(ev_now(loop) + timeout),
            _interval(0)
        {
            // store the object in the data "void*"
            _timer.data = this;
            
            // initialize the libev structure, it should expire after the connection timeout
            ev_timer_init(&_timer, callback, timeout, 0.0);

            // start the timer (this is the time that we reserve for setting up the connection)
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
         *  Start the timer (and expose the interval)
         *  @param  interval        the heartbeat interval proposed by the server
         *  @return uint16_t        the heartbeat interval that we accepted
         */
        uint16_t start(uint16_t interval)
        {
            // we now know for sure that the connection was set up
            _interval = interval;
            
            // if heartbeats are disabled we do not have to set it
            if (_interval == 0) return 0;
            
            // calculate current time
            auto now = ev_now(_loop);
            
            // we also know when the next heartbeat should be sent
            _next = now + std::max(1, _interval / 2);

            // find the earliest thing that expires
            // @todo does this work?
            _timer.repeat = std::min(_next, _expire) - now;
            
            // restart the timer
            ev_timer_again(_loop, &_timer);
            
            // expose the accepted interval
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
                _watchers.remove_if([fd](const Watcher &watcher) -> bool {
                    
                    // compare filedescriptors
                    return watcher.contains(fd);
                });
            }
            else
            {
                // look in the array for this filedescriptor
                for (auto &watcher : _watchers)
                {
                    // do we have a match?
                    if (watcher.contains(fd)) return watcher.events(events);
                }
                
                // we need a watcher
                Watchable *watchable = this;
                
                // we should monitor a new filedescriptor
                _watchers.emplace_back(_loop, watchable, fd, events);
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
     *  @var std::list
     */
    std::list<Wrapper> _wrappers;

    /**
     *  Lookup a connection-wrapper, when the wrapper is not found, we construct one
     *  @param  connection
     *  @return Wrapper
     */
    Wrapper &lookup(TcpConnection *connection)
    {
        // look for the appropriate connection
        for (auto &wrapper : _wrappers)
        {
            // do we have a match?
            if (wrapper.contains(connection)) return wrapper;
        }
        
        // add to the wrappers
        _wrappers.emplace_back(_loop, connection);
        
        // done
        return _wrappers.back();
    }

    /**
     *  Method that is called by AMQP-CPP to register a filedescriptor for readability or writability
     *  @param  connection  The TCP connection object that is reporting
     *  @param  fd          The filedescriptor to be monitored
     *  @param  flags       Should the object be monitored for readability or writability?
     */
    virtual void monitor(TcpConnection *connection, int fd, int flags) override final
    {
        // lookup the appropriate wrapper and start monitoring
        lookup(connection).monitor(fd, flags);
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
        // lookup the wrapper, and start the timer to check for activity and send heartbeats
        return lookup(connection).start(interval);
    }

    /**
     *  Method that is called when the TCP connection is destructed
     *  @param  connection  The TCP connection
     */
    virtual void onDetached(TcpConnection *connection) override
    {
        // remove from the array
        _wrappers.remove_if([connection](const Wrapper &wrapper) -> bool {
            return wrapper.contains(connection);
        });
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
