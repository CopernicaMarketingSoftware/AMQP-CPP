/**
 *  LibBoostAsio.h
 *
 *  Implementation for the AMQP::TcpHandler for boost::asio. You can use this class
 *  instead of a AMQP::TcpHandler class, just pass the boost asio service to the
 *  constructor and you're all set.  See tests/libboostasio.cpp for example.
 *
 *  Watch out: this class was not implemented or reviewed by the original author of
 *  AMQP-CPP. However, we do get a lot of questions and issues from users of this class,
 *  so we cannot guarantee its quality. If you run into such issues too, it might be
 *  better to implement your own handler that interact with boost.
 *
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
#include <memory>

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/bind/bind.hpp>
#include <boost/function.hpp>

#include "amqpcpp/linux_tcp.h"

// C++17 has 'weak_from_this()' support.
#if __cplusplus >= 201701L
#define PTR_FROM_THIS(T) weak_from_this()
#else
#define PTR_FROM_THIS(T) std::weak_ptr<T>(shared_from_this())
#endif

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
protected:

    /**
     *  Helper class that wraps a boost io_context socket monitor.
     */
    class Watcher : public virtual std::enable_shared_from_this<Watcher>
    {
    private:

        using strand_weak_ptr = std::weak_ptr<boost::asio::io_context::strand>;

        /**
         *  The boost asio io_context::strand managed pointer.
         *  @var class std::shared_ptr<boost::asio::io_context>
         */
        strand_weak_ptr _wpstrand;

        /**
         *  The boost tcp socket.
         *  @var class boost::asio::ip::tcp::socket
         *  @note https://stackoverflow.com/questions/38906711/destroying-boost-asio-socket-without-closing-native-handler
         */
        boost::asio::posix::stream_descriptor _socket;

        /**
         *  The boost asynchronous deadline timer for heartbeats.
         *  @var class boost::asio::deadline_timer
         */
        boost::asio::deadline_timer _heartbeat_timer;

        /**
         *  The boost asynchronous deadline timer for read event timeouts.
         *  @var class boost::asio::deadline_timer
         */
        boost::asio::deadline_timer _expire_timer;

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
         *  The timeout in seconds between each heartbeat to be sent.
         *  @var _timeout 0 if no timeout is agreed.
         */
        uint16_t _heartbeat_interval{0};

        /**
         *  The timeout in seconds negotiated with the server.
         *  @var _timeout 0 if no timeout is agreed.
         */
        uint16_t _expire_timeout{0};

        using handler = boost::function<void(boost::system::error_code)>;
        using io_handler = boost::function<void(boost::system::error_code, const std::size_t)>;

        /**
         * Builds a io handler callback that executes the io callback in a strand.
         * @param  io_handler  The handler callback to dispatch
         * @return io_handler  A function wrapping the execution of the handler function in a io_context::strand.
         */
        io_handler get_dispatch_wrapper(const handler& fn)
        {
            const strand_weak_ptr wpstrand = _wpstrand;
            return [fn, wpstrand] (const boost::system::error_code &ec, const std::size_t transfered_bytes)
            {
                const strand_shared_ptr strand = wpstrand.lock();
                if (!strand)
                {
                    fn(boost::system::errc::make_error_code(boost::system::errc::operation_canceled));
                    return;
                }
                boost::asio::dispatch(strand->context().get_executor(), boost::bind(fn, ec));
            };
        }

        /**
         * Binds and returns a read handler for the io operation.
         * @param  connection   The connection being watched.
         * @param  fd           The file descripter being watched.
         * @return handler callback
         */
        io_handler get_read_handler(TcpConnection *const connection, const int fd)
        {
            auto fn = boost::bind(&Watcher::read_handler,
                                  this,
                                  boost::placeholders::_1,
                                  PTR_FROM_THIS(Watcher),
                                  connection,
                                  fd);
            return get_dispatch_wrapper(fn);
        }

        /**
         * Binds and returns a read handler for the io operation.
         * @param  connection   The connection being watched.
         * @param  fd           The file descripter being watched.
         * @return handler callback
         */
        io_handler get_write_handler(TcpConnection *const connection, const int fd)
        {
            auto fn = boost::bind(&Watcher::write_handler,
                                  this,
                                  boost::placeholders::_1,
                                  PTR_FROM_THIS(Watcher),
                                  connection,
                                  fd);
            return get_dispatch_wrapper(fn);
        }

        /**
         * Binds and returns a lamba function handler for the heartbeat operation.
         * @param  connection   The connection being watched.
         * @return handler callback
         */
        handler get_heartbeat_handler(TcpConnection *const connection)
        {
            const auto fn = boost::bind(&Watcher::heartbeat_handler,
                                        this,
                                        boost::placeholders::_1,
                                        PTR_FROM_THIS(Watcher),
                                        connection);
            return boost::bind(get_dispatch_wrapper(fn), boost::placeholders::_1, 0);
        }

        /**
         * Binds and returns a lamba function handler for the expire operation.
         * @param  connection   The connection being watched.
         * @return handler callback
         */
        handler get_expire_handler(TcpConnection *const connection)
        {
            const auto fn = boost::bind(&Watcher::expire_handler,
                                        this,
                                        boost::placeholders::_1,
                                        PTR_FROM_THIS(Watcher),
                                        connection);
            return boost::bind(get_dispatch_wrapper(fn), boost::placeholders::_1, 0);
        }

        /**
         *  Handler method that is called by boost's io_context when the socket pumps a read event.
         *  @param  ec          The status of the callback.
         *  @param  awpWatcher  A weak pointer to this object.
         *  @param  connection  The connection being watched.
         *  @param  fd          The file descriptor being watched.
         *  @note   The handler will get called if a read is cancelled.
         */
        void read_handler(const boost::system::error_code &ec,
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
                // if the server is readable, we have some extra time before it expires, the expire time
                // is set to 1.5 * _timeout to close the connection when the third heartbeat is about to be sent
                if (_expire_timeout > 0)
                {
                    _expire_timer.expires_from_now(boost::posix_time::seconds(_expire_timeout));
                    _expire_timer.async_wait(get_expire_handler(connection));
                }

                connection->process(fd, AMQP::readable);

                _read_pending = true;

                _socket.async_read_some(
                    boost::asio::null_buffers(),
                    get_read_handler(connection, fd));
            }
        }

        /**
         *  Handler method that is called by boost's io_context when the socket pumps a write event.
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

                _socket.async_write_some(
                    boost::asio::null_buffers(),
                    get_write_handler(connection, fd));
            }
        }

        /**
         *  Callback method that is called by libev when the timer expires
         *  @param  ec          error code returned from loop
         *  @param  awpWatcher  A weak pointer to this object.
         *  @param  connection
         *  @param  timeout
         */
        void heartbeat_handler(const boost::system::error_code &ec,
                               std::weak_ptr<Watcher> awpThis,
                               TcpConnection *const connection)
        {
            // Resolve any potential problems with dangling pointers
            // (remember we are using async).
            const std::shared_ptr<Watcher> apTimer = awpThis.lock();
            if (!apTimer) { return; }

            if (!ec)
            {
                if (connection)
                {
                    // send the heartbeat
                    connection->heartbeat();
                }

                // Reschedule the timer and register handler
                _heartbeat_timer.expires_from_now(boost::posix_time::seconds(_heartbeat_interval));
                _heartbeat_timer.async_wait(get_heartbeat_handler(connection));
            }
        }

        /**
         *  Callback method that is called by libev when the timer expires
         *  @param  ec          error code returned from loop
         *  @param  awpWatcher  A weak pointer to this object.
         *  @param  connection
         */
        void expire_handler(const boost::system::error_code &ec,
                            std::weak_ptr<Watcher> awpThis,
                            TcpConnection *const connection)
        {
            // Resolve any potential problems with dangling pointers
            // (remember we are using async).
            const std::shared_ptr<Watcher> apTimer = awpThis.lock();
            if (!apTimer) { return; }
            if (!ec)
            {
                if (connection)
                {
                    // this is a connection timeout, close the connection from our side too
                    connection->close(true);
                }
            }
        }

    public:
        /**
         *  Constructor- initialises the watcher and assigns the filedescriptor to
         *  a boost socket for monitoring.
         *  @param  io_context      The boost io_context
         *  @param  wpstrand        A weak pointer to a io_context::strand instance.
         *  @param  fd              The filedescriptor being watched
         */
        Watcher(boost::asio::io_context &io_context,
                const strand_weak_ptr wpstrand,
                const int fd) :
            _wpstrand(wpstrand),
            _socket(io_context),
            _heartbeat_timer(io_context),
            _expire_timer(io_context)
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
            stop_timers();
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

                _socket.async_read_some(
                    boost::asio::null_buffers(),
                    get_read_handler(connection, fd));
            }

            // 2. Handle writes?
            _write = ((events & AMQP::writable) != 0);

            // Write requested but no write pending?
            if (_write && !_write_pending)
            {
                _write_pending = true;

                _socket.async_write_some(
                    boost::asio::null_buffers(),
                    get_write_handler(connection, fd));
            }
        }

        /**
         *  Change the expire and heartbeat timers
         *  @param  connection
         *  @param  timeout
         */
        void set_timers(TcpConnection *connection, uint16_t heartbeat_timeout)
        {
            // stop timer in case it was already set
            stop_timers();

            // timeout not applicable
            if (heartbeat_timeout == 0) { return; }

            // save new interval
            _heartbeat_interval = heartbeat_timeout * 0.5;

            // Reschedule the timer and register handler
            _heartbeat_timer.expires_from_now(boost::posix_time::seconds(_heartbeat_interval));
            _heartbeat_timer.async_wait(get_heartbeat_handler(connection));

            // save new timeout - increase the agreed timeout to be a bit permissive
            _expire_timeout = heartbeat_timeout * 1.5;

            // Reschedule the timer and register handler
            _expire_timer.expires_from_now(boost::posix_time::seconds(_expire_timeout));
            _expire_timer.async_wait(get_expire_handler(connection));
        }

        /**
         *  Stop the timer
         */
        void stop_timers()
        {
            // do nothing if it was never set
            _heartbeat_timer.cancel();
            _expire_timer.cancel();
        }
    };

    /**
     *  The boost asio io_context.
     *  @var class boost::asio::io_context&
     */
    boost::asio::io_context & _iocontext;

    using strand_shared_ptr = std::shared_ptr<boost::asio::io_context::strand>;

    /**
     *  The boost asio io_context::strand managed pointer.
     *  @var class std::shared_ptr<boost::asio::io_context>
     */
    strand_shared_ptr _strand;

    /**
     *  All I/O watchers that are active, indexed by their filedescriptor
     *  @var std::map<int,Watcher>
     */
    std::map<int, std::shared_ptr<Watcher> > _watchers;

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
                std::make_shared<Watcher>(_iocontext, _strand, fd);
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
     *  @param  timeout         The suggested timeout from the server
     *  @return uint16_t        The timeout to use
     */
    virtual uint16_t onNegotiate(TcpConnection *connection, uint16_t timeout) override
    {
        // skip if no heartbeats are needed
        if (timeout == 0) return 0;

        const auto fd = connection->fileno();

        auto iter = _watchers.find(fd);
        if (iter == _watchers.end()) return 0;

        // set the timer
        iter->second->set_timers(connection, timeout);

        // we agree with the timeout
        return timeout;
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
     *  @param  io_context    The boost io_context to wrap
     */
    explicit LibBoostAsioHandler(boost::asio::io_context &io_context) :
        _iocontext(io_context),
        _strand(std::make_shared<boost::asio::io_context::strand>(_iocontext))
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
     *  Returns a reference to the boost io_context object that is being used.
     *  @return The boost io_context object.
     */
    boost::asio::io_context &service()
    {
        return _iocontext;
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
