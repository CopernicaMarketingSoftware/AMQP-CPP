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
 *  @copyright 2015 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <ev.h>

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
            // retrieve the connection
            TcpConnection *connection = static_cast<TcpConnection*>(watcher->data);

            // tell the connection that its filedescriptor is active
            connection->process(watcher->fd, revents);
        }

    public:
        /**
         *  Constructor
         *  @param  loop            The current event loop
         *  @param  connection      The connection being watched
         *  @param  fd              The filedescriptor being watched
         *  @param  events          The events that should be monitored
         */
        Watcher(struct ev_loop *loop, TcpConnection *connection, int fd, int events) : _loop(loop)
        {
            // initialize the libev structure
            ev_io_init(&_io, callback, fd, events);

            // store the connection in the data "void*"
            _io.data = connection;

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
     *  The event loop
     *  @var struct ev_loop*
     */
    struct ev_loop *_loop;

    /**
     *  All I/O watchers that are active, indexed by their filedescriptor
     *  @var std::map<int,Watcher>
     */
    std::map<int,std::unique_ptr<Watcher>> _watchers;


    /**
     *  Method that is called by AMQP-CPP to register a filedescriptor for readability or writability
     *  @param  connection  The TCP connection object that is reporting
     *  @param  fd          The filedescriptor to be monitored
     *  @param  flags       Should the object be monitored for readability or writability?
     */
    virtual void monitor(TcpConnection *connection, int fd, int flags) override
    {
        // do we already have this filedescriptor
        auto iter = _watchers.find(fd);

        // was it found?
        if (iter == _watchers.end())
        {
            // we did not yet have this watcher - but that is ok if no filedescriptor was registered
            if (flags == 0) return;

            // construct a new watcher, and put it in the map
            _watchers[fd] = std::unique_ptr<Watcher>(new Watcher(_loop, connection, fd, flags));
        }
        else if (flags == 0)
        {
            // the watcher does already exist, but we no longer have to watch this watcher
            _watchers.erase(iter);
        }
        else
        {
            // change the events
            iter->second->events(flags);
        }
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
