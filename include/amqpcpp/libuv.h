/**
 *  LibUV.h
 *
 *  Implementation for the AMQP::TcpHandler that is optimized for libuv. You can
 *  use this class instead of a AMQP::TcpHandler class, just pass the event loop
 *  to the constructor and you're all set.
 *
 *  Based heavily on the libev.h implementation by Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *
 *  @author David Nikdel <david@nikdel.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <uv.h>
#include <list>
#include "amqpcpp/linux_tcp.h"

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class LibUvHandler : public TcpHandler
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
         *  @var uv_loop_t
         */
        uv_loop_t *_loop;

        /**
         *  The actual watcher structure
         *  @var uv_poll_t
         */
        uv_poll_t *_poll;


        /**
         * monitoring file descriptor
        */
        int _fd;

        /**
         *  Callback method that is called by libuv when a filedescriptor becomes active
         *  @param  handle     Internal poll handle
         *  @param  status     LibUV error code UV_*
         *  @param  events     Events triggered
         */
        static void callback(uv_poll_t *handle, int status, int events)
        {
            // retrieve the connection
            Watchable *obj = static_cast<Watchable*>(handle->data);

            // tell the connection that its filedescriptor is active
            int fd = -1;
            uv_fileno(reinterpret_cast<uv_handle_t*>(handle), &fd);
            obj->onActive(fd, uv_to_amqp_events(status, events));

        }


        /**
         *  Convert event flags from UV format to AMQP format
         */
        static int uv_to_amqp_events(int status, int events)
        {
            // if the socket is closed report both so we pick up the error
            if (status != 0)
                return AMQP::readable | AMQP::writable;

            // map read or write
            int amqp_events = 0;
            if (events & UV_READABLE)
                amqp_events |= AMQP::readable;
            if (events & UV_WRITABLE)
                amqp_events |= AMQP::writable;
            return amqp_events;
        }

        /**
         *  Convert event flags from AMQP format to UV format
         */
        static int amqp_to_uv_events(int events)
        {
            int uv_events = 0;
            if (events & AMQP::readable)
                uv_events |= UV_READABLE;
            if (events & AMQP::writable)
                uv_events |= UV_WRITABLE;
            return uv_events;
        }


    public:
        /**
         *  Constructor
         *  @param  loop            The current event loop
         *  @param  obj             The watchable object being watched
         *  @param  fd              The filedescriptor being watched
         *  @param  events          The events that should be monitored
         */
        Watcher(uv_loop_t *loop, Watchable *obj, int fd, int events) : 
            _loop(loop),
            _fd(fd)
        {
            // create a new poll
            _poll = new uv_poll_t();

            // initialize the libev structure
            uv_poll_init(_loop, _poll, fd);

            // store the connection in the data "void*"
            _poll->data = obj;

            // start the watcher
            uv_poll_start(_poll, amqp_to_uv_events(events), callback);
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
            uv_poll_stop(_poll);

            // close the handle
            uv_close(reinterpret_cast<uv_handle_t*>(_poll), [](uv_handle_t* handle) {
                // delete memory once closed
                delete reinterpret_cast<uv_poll_t*>(handle);
            });
        }

        /**
         *  Change the events for which the filedescriptor is monitored
         *  @param  events
         */
        void events(int events)
        {
            // update the events being watched for
            uv_poll_start(_poll, amqp_to_uv_events(events), callback);
        }

        /**
         *  Check if a filedescriptor is covered by the watcher
         *  @param  fd
         *  @return bool
         */
        bool contains (int fd) const {
            return fd == _fd;
        }
    };


    class UvConnectionWrapper : private Watchable
    {
    private:

        uv_loop_t* _loop;
        uv_timer_t _timer;
        AMQP::TcpConnection* _conn;
        std::list<Watcher> _watchers;
        uint64_t _timeout;
        uint64_t _expire;
        uint64_t _next;

        /**
         *  timer callback
         *  @param  handle  Internal timer handle
         */
        static void timer_cb(uv_timer_t* handle)
        {
            // retrieving the connection
            Watchable* obj = static_cast<Watchable*>(handle->data);

            // telliing the connection to send a heartbeat to the broker
            obj->onExpired();
        }

        bool timed() const
        {
            // if neither timers are set
            return _expire > 0.0 || _next > 0.0;
        }
        

        virtual void onExpired() override
        {
            // get the current time
            auto now = uv_now(_loop);
            
            // timer is no longer active, so the refcounter in the loop is restored
            // uv_ref(_loop);

            // if the onNegotiate method was not yet called, and no heartbeat timeout was negotiated
            if (_timeout == 0)
            {
                // this can happen in three situations: 1. a connect-timeout, 2. user space has
                // told us that we're not interested in heartbeats, 3. rabbitmq does not want heartbeats,
                // in either case we're no longer going to run further timers.
                _next = _expire = 0.0;
                
                // if we have an initialized connection, user-space must have overridden the onNegotiate
                // method, so we keep using the connection
                if (_conn->initialized()) return;

                // this is a connection timeout, close the connection from our side too
                return (void)_conn->close(true);
            }
            else if (now >= _expire)
            {
                // the server was inactive for a too long period of time, reset state
                _next = _expire = 0.0; _timeout = 0;
                
                // close the connection because server was inactive (we close it with immediate effect,
                // because it was inactive so we cannot trust it to respect the AMQP close handshake)
                return (void)_conn->close(true);
            }
            else if (now >= _next)
            {
                // it's time for the next heartbeat
                _conn->heartbeat();
                
                // remember when we should send out the next one, so the next one should be 
                // sent only after _timout/2 seconds again _from now_ (no catching up)
                _next = now + std::max(_timeout / 2, static_cast<u_int64_t>(1000));
            }

            // reset the timer to trigger again later
            uv_timer_start(&_timer, timer_cb, std::min(_next, _expire) - now, 0.0);

            // and start it again
            // ev_timer_start(_loop, &_timer);
            
            // and because the timer is running again, we restore the refcounter
            uv_unref((uv_handle_t*)_loop);
        }

        /**
         *  Method that is called when a filedescriptor becomes active
         *  @param  fd          the filedescriptor that is active
         *  @param  events      the events that are active (readable/writable)
         */
        virtual void onActive(int fd, int events) override
        {
            // if the server is readable, we have some extra time before it expires, the expire time 
            // is set to 1.5 * _timeout to close the connection when the third heartbeat is about to be sent
            if (_timeout != 0 && (events & UV_READABLE)) _expire = uv_now(_loop) + _timeout * 1.5;
            
            // pass on to the connection
            _conn->process(fd, events);
        }

        uint64_t sec_to_ms(uint64_t sec)
        {
            if(sec > 0) return sec * 1000; 
            return 0;
        }        

        uint16_t ms_to_sec(uint64_t millis)
        {
            if(millis > 0) return millis / 1000;
            return 0;
        }

    public:
        UvConnectionWrapper(uv_loop_t* loop, AMQP::TcpConnection* connection, uint64_t timeout) :
            _loop(loop),
            _conn(connection),
            _timeout(sec_to_ms(timeout)),
            _next(0),
            _expire(uv_now(_loop) + sec_to_ms(timeout))
        {
            _timer.data = this;
            uv_timer_init(_loop, &_timer);
            uv_timer_start(&_timer, timer_cb, _timeout, 0.0);
            // uv_unref((uv_handle_t*)_loop);
        }

        UvConnectionWrapper(UvConnectionWrapper &&that) = delete;
        UvConnectionWrapper(const UvConnectionWrapper &&that) = delete;

        virtual ~UvConnectionWrapper()
        {
            if(!timed()) return;

            // stop the timer
            uv_timer_stop(&_timer);

            // restore loop refcount
            // uv_ref((uv_handle_t*)_loop);            
        }

        bool contains(AMQP::TcpConnection* connection) const 
        {
            return _conn == connection;
        }


        uint16_t start(uint16_t timeout)
        {
            // we now know for sure that the connection was set up
            _timeout = sec_to_ms(timeout);
            
            // if heartbeats are disabled we do not have to set it
            if (_timeout == 0) return 0;
            
            // calculate current time
            auto now = uv_now(_loop);
            
            // we also know when the next heartbeat should be sent
            _next = now + std::max(_timeout / 2, static_cast<u_int64_t>(1000));
            
            // because the server has just sent us some data, we will update the expire time too
            _expire = now + _timeout * 1.5;

            // stop the existing timer (we have to stop it and restart it, because ev_timer_set() 
            // on its own does not change the running timer) (note that we assume that the timer
            // is already running and keeps on running, so no calls to ev_ref()/en_unref() here)
            uv_timer_stop(&_timer);

            // find the earliest thing that expires
            uv_timer_start(&_timer, timer_cb, std::min(_next, _expire) - now, 0.0);

            // and start it again
            // ev_timer_start(_loop, &_timer);
            
            // expose the accepted interval
            return ms_to_sec(_timeout);
        }


        /**
         *  Monitor a filedescriptor
         *  @param  fd          specific file discriptor
         *  @param  events      updated events
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
                
                // if not found we need to register a new filedescriptor
                Watchable *watchable = this;
                
                // we should monitor a new filedescriptor
                _watchers.emplace_back(_loop, watchable, fd, events);
            }
        }            
    };


    /**
     *  The event loop
     *  @var uv_loop_t*
     */
    uv_loop_t *_loop;


    std::list<UvConnectionWrapper> _wrappers;

    UvConnectionWrapper &lookup(TcpConnection *connection)
    {
        // look for the appropriate connection
        for (auto &wrapper : _wrappers)
        {
            // do we have a match?
            if (wrapper.contains(connection)) return wrapper;
        }
        
        // add to the wrappers
        _wrappers.emplace_back(_loop, connection, 60);
        
        // done
        return _wrappers.back();
    }

protected:
    /**
     *  Method that is called by AMQP-CPP to register a filedescriptor for readability or writability
     *  @param  connection  The TCP connection object that is reporting
     *  @param  fd          The filedescriptor to be monitored
     *  @param  flags       Should the object be monitored for readability or writability?
     */
    virtual void monitor(TcpConnection *connection, int fd, int flags) override
    {
        // lookup the appropriate wrapper and start monitoring
        lookup(connection).monitor(fd, flags);
    }

    /**
     *  Method that is called when the server sends a heartbeat to the client
     *  @param  connection  The connection over which the heartbeat was received
     *  @param  interval    agreed interval by the broker  
     *  @see    ConnectionHandler::onHeartbeat
     */
    virtual uint16_t onNegotiate(TcpConnection *connection, uint16_t interval) override
    {
        // lookup the wrapper, and start the timer to check for activity and send heartbeats
        return lookup(connection).start(interval);

    }   

    virtual void onDetached(TcpConnection *connection) override
    {
        // remove from the array
        _wrappers.remove_if([connection](const UvConnectionWrapper &wrapper) -> bool {
            return wrapper.contains(connection);
        });
    }

public:
    /**
     *  Constructor
     *  @param  loop    The event loop to wrap
     */
    LibUvHandler(uv_loop_t *loop) : _loop(loop) {}

    /**
     *  Destructor
     */
    ~LibUvHandler() = default;
};

/**
 *  End of namespace
 */
}
