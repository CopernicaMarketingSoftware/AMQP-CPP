/**
 *  LibBoostAsio.h
 *
 *  Implementation for the AMQP::TcpHandler that is optimized for boost::asio. You can
 *  use this class instead of a AMQP::TcpHandler class, just pass the boost asio service
 *  to the constructor and you're all set
 *
 *  @author Gavin Smith <gavin.smith@coralbay.tv>
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <boost/asio/io_service.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>


// boost::function< void(boost::system::error_code ec) > f = 
#define STRAND_HANDLER(_fn)                                                                     \
[this, fn = _fn, strand = _strand](const boost::system::error_code ec)                          \
{                                                                                               \
    const std::shared_ptr<boost::asio::io_service::strand> apStrand = strand.lock();            \
    if (!apStrand) { return; }                                                                  \
                                                                                                \
    apStrand->dispatch(boost::bind(fn,ec));                                                     \
}

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 *  @note Because of a limitation on Windows, this will only work on POSIX based systems - see https://github.com/chriskohlhoff/asio/issues/70
 */
class LibBoostAsioHandler : public virtual TcpHandler
{
private:

    /**
     *  Helper class that wraps a boost io_service socket monitor.
     */
    class Watcher : public virtual std::enable_shared_from_this<Watcher>
    {
    private:

        /**
         *  The boost asio io_service which is responsible for detecting events.
         *  @var class boost::asio::io_service&
         */
        boost::asio::io_service & _ioservice;

        /**
         *  The boost tcp socket.
         *  @var class boost::asio::ip::tcp::socket
         *  @note https://stackoverflow.com/questions/38906711/destroying-boost-asio-socket-without-closing-native-handler
         */
        boost::asio::posix::stream_descriptor _socket;


        /**
         *  A boolean that indicates if the watcher is monitoring for read events.
         *  @var _read True if reads are being monitored else false.
         */
        bool _read{false};

        /**
         *  A boolean that indicates if the watcher has a pending read event.
         *  @var _read True if read is pending else false.
         */
        bool _read_pending{false};

        /**
         *  A boolean that indicates if the watcher is monitoring for write events.
         *  @var _read True if writes are being monitored else false.
         */
        bool _write{false};

        /**
         *  A boolean that indicates if the watcher has a pending write event.
         *  @var _read True if read is pending else false.
         */
        bool _write_pending{false};

        /**
         *  Handler method that is called by boost's io_service when the socket pumps a read event.
         *  @param  ec          The status of the callback.
         *  @param  awpWatcher  A weak pointer to this object.
         *  @param  connection  The connection being watched.
         *  @param  fd          The file descriptor being watched.
         *  @note   The handler will get called if a read is cancelled.
         */
        void read_handler(const boost::system::error_code ec,
                          const std::weak_ptr<Watcher> awpWatcher,
                          TcpConnection *const connection,
                          const int fd)
        {
	        // Resolve any potential problems with dangling pointers
            // (remember we are using async).
            const std::shared_ptr<Watcher> apWatcher = awpWatcher.lock();
            if (!apWatcher) { return; }

            _read_pending = false;

            if ((!ec || ec == boost::asio::error::would_block) && _read)
            {
                connection->process(fd, AMQP::readable);

                _read_pending = true;

                _socket.async_read_some(boost::asio::null_buffers(),
                                        boost::bind(&Watcher::read_handler,
                                                    this,
                                                    boost::placeholders::_1,
// C++17 has 'weak_from_this()' support.
#if __cplusplus >= 201701L
                                                    weak_from_this()
#else
                                                    shared_from_this(),
#endif
                                                    connection,
                                                    fd));
            }
        }

        /**
         *  Handler method that is called by boost's io_service when the socket pumps a write event.
         *  @param  ec          The status of the callback.
         *  @param  awpWatcher  A weak pointer to this object.
         *  @param  connection  The connection being watched.
         *  @param  fd          The file descriptor being watched.
         *  @note   The handler will get called if a write is cancelled.
         */
        void write_handler(const boost::system::error_code ec,
                           const std::weak_ptr<Watcher> awpWatcher,
                           TcpConnection *const connection,
                           const int fd)
        {
	        // Resolve any potential problems with dangling pointers
            // (remember we are using async).
            const std::shared_ptr<Watcher> apWatcher = awpWatcher.lock();
            if (!apWatcher) { return; }

            _write_pending = false;

            if ((!ec || ec == boost::asio::error::would_block) && _write)
            {
                connection->process(fd, AMQP::writable);

                _write_pending = true;

                _socket.async_write_some(boost::asio::null_buffers(),
                                         boost::bind(&Watcher::write_handler,
                                                     this,
                                                     boost::placeholders::_1,
// C++17 has 'weak_from_this()' support.
#if __cplusplus >= 201701L
                                                     weak_from_this()
#else
                                                     shared_from_this(),
#endif
                                                     connection,
                                                     fd));
            }
        }

    public:
        /**
         *  Constructor- initialises the watcher and assigns the filedescriptor to 
         *  a boost socket for monitoring.
         *  @param  io_service      The boost io_service
         *  @param  fd              The filedescriptor being watched
         */
        Watcher(boost::asio::io_service &io_service, const int fd) :
            _ioservice(io_service),
            _socket(_ioservice)
        {
            _socket.assign(fd);

            _socket.non_blocking(true);
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
        ~Watcher()
        {
            _read = false;
            _write = false;
            _socket.release();
        }

        /**
         *  Change the events for which the filedescriptor is monitored
         *  @param  events
         */
        void events(TcpConnection *connection, int fd, int events)
        {
            // 1. Handle reads?
            _read = ((events & AMQP::readable) != 0);

            // Read requsted but no read pending?
            if (_read && !_read_pending)
            {
                _read_pending = true;

                _socket.async_read_some(boost::asio::null_buffers(),
                                        boost::bind(&Watcher::read_handler,
                                                    this,
                                                    boost::placeholders::_1,
// C++17 has 'weak_from_this()' support.
#if __cplusplus >= 201701L
                                                    weak_from_this()
#else
                                                    shared_from_this(),
#endif
                                                    connection,
                                                    fd));
            }

            // 2. Handle writes?
            _write = ((events & AMQP::writable) != 0);

            // Write requested but no write pending?
            if (_write && !_write_pending)
            {
                _write_pending = true;

                _socket.async_write_some(boost::asio::null_buffers(),
                                         boost::bind(&Watcher::write_handler,
                                                     this,
                                                     boost::placeholders::_1,
// C++17 has 'weak_from_this()' support.
#if __cplusplus >= 201701L
                                                     weak_from_this()
#else
                                                     shared_from_this(),
#endif
                                                     connection,
                                                     fd));
            }
        }
    };

    /**
     *  Timer class to periodically fire a heartbeat
     */
    class Timer : public std::enable_shared_from_this<Timer>
    {
    private:

        /**
         *  The boost asio io_service which is responsible for detecting events.
         *  @var class boost::asio::io_service&
         */
        boost::asio::io_service & _ioservice;
        
        /**
         *  The boost asynchronous deadline timer.
         *  @var class boost::asio::deadline_timer
         */
        boost::asio::deadline_timer _timer;

        /**
         *  Callback method that is called by libev when the timer expires
         *  @param  loop        The loop in which the event was triggered
         *  @param  timer       Internal timer object
         *  @param  revents     The events that triggered this call
         */
        void timeout(const boost::system::error_code &ec,
                     std::weak_ptr<Timer> awpThis,
                     TcpConnection *const connection,
                     const uint16_t timeout)
        {
            // Resolve any potential problems with dangling pointers
            // (remember we are using async).
            const std::shared_ptr<Timer> apTimer = awpThis.lock();
            if (!apTimer) { return; }

            if (!ec)
            {
                if (connection)
                {
                    // send the heartbeat
                    connection->heartbeat();
                }

                // Reschedule the timer for the future:
                _timer.expires_at(_timer.expires_at() + boost::posix_time::seconds(timeout));

                // Posts the timer event
                _timer.async_wait(boost::bind(&Timer::timeout,
                                              this,
                                              boost::placeholders::_1,
// C++17 has 'weak_from_this()' support.
#if __cplusplus >= 201701L
                                              weak_from_this()
#else
                                              shared_from_this(),
#endif
                                              connection,
                                              timeout));
            }
        }

        /**
         *  Stop the timer
         */
        void stop()
        {
            // do nothing if it was never set
            _timer.cancel();
        }

    public:
        /**
         *  Constructor
         *  @param  loop            The current event loop
         */
        Timer(boost::asio::io_service &io_service) :
            _ioservice(io_service),
            _timer(_ioservice)
        {

        }

        /**
         *  Timers cannot be copied or moved
         *
         *  @param  that    The object to not move or copy
         */
        Timer(Timer &&that) = delete;
        Timer(const Timer &that) = delete;

        /**
         *  Destructor
         */
        ~Timer()
        {
            // stop the timer
            stop();
        }

        /**
         *  Change the expire time
         *  @param  connection
         *  @param  timeout
         */
        void set(TcpConnection *connection, uint16_t timeout)
        {
            // stop timer in case it was already set
            stop();

            _timer.expires_from_now(boost::posix_time::seconds(timeout));
            _timer.async_wait(boost::bind(&Timer::timeout,
                                          this,
                                          boost::placeholders::_1,
// C++17 has 'weak_from_this()' support.
#if __cplusplus >= 201701L
                                          weak_from_this()
#else
                                          shared_from_this(),
#endif
                                          connection, 
                                          timeout));
        }
    };

    /**
     *  The boost asio io_service.
     *  @var class boost::asio::io_service&
     */
    boost::asio::io_service & _ioservice;


    /**
     *  All I/O watchers that are active, indexed by their filedescriptor
     *  @var std::map<int,Watcher>
     */
    std::map<int, std::shared_ptr<Watcher> > _watchers;


    std::shared_ptr<Timer> _timer;


    /**
     *  Method that is called by AMQP-CPP to register a filedescriptor for readability or writability
     *  @param  connection  The TCP connection object that is reporting
     *  @param  fd          The filedescriptor to be monitored
     *  @param  flags       Should the object be monitored for readability or writability?
     */
    void monitor(TcpConnection *const connection,
                 const int fd,
                 const int flags) override
    {
        // do we already have this filedescriptor
        auto iter = _watchers.find(fd);

        // was it found?
        if (iter == _watchers.end())
        {
            // we did not yet have this watcher - but that is ok if no filedescriptor was registered
            if (flags == 0){ return; }

            // construct a new pair (watcher/timer), and put it in the map
            const std::shared_ptr<Watcher> apWatcher = 
				std::make_shared<Watcher>(_ioservice, fd);

            _watchers[fd] = apWatcher;
            
            // explicitly set the events to monitor
            apWatcher->events(connection, fd, flags);
        }
        else if (flags == 0)
        {
            // the watcher does already exist, but we no longer have to watch this watcher
            _watchers.erase(iter);
        }
        else
        {
            // Change the events on which to act.
            iter->second->events(connection,fd,flags);
        }
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
        // skip if no heartbeats are needed
        if (interval == 0) return 0;

        // set the timer
        _timer->set(connection, interval);

        // we agree with the interval
        return interval;
    }

public:

    /**
     *  Handler cannot be default constructed.
     *
     *  @param  that    The object to not move or copy
     */
    LibBoostAsioHandler() = delete;

    /**
     *  Constructor
     *  @param  io_service    The boost io_service to wrap
     */
    explicit LibBoostAsioHandler(boost::asio::io_service &io_service) :
        _ioservice(io_service),
        _timer(std::make_shared<Timer>(_ioservice))
    {

    }

    /**
     *  Handler cannot be copied or moved
     *
     *  @param  that    The object to not move or copy
     */
    LibBoostAsioHandler(LibBoostAsioHandler &&that) = delete;
    LibBoostAsioHandler(const LibBoostAsioHandler &that) = delete;

    /**
     *  Returns a reference to the boost io_service object that is being used.
     *  @return The boost io_service object.
     */
    boost::asio::io_service &service()
    {
       return _ioservice;
    }

    /**
     *  Destructor
     */
    ~LibBoostAsioHandler() override = default;
};


/**
 *  End of namespace
 */
}

