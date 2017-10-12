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
        void read_handler(boost::system::error_code ec,
                          std::weak_ptr<Watcher> awpWatcher,
                          TcpConnection *connection,
                          int fd)
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
        void write_handler(boost::system::error_code ec,
                           std::weak_ptr<Watcher> awpWatcher,
                           TcpConnection *connection,
                           int fd)
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
        Watcher(boost::asio::io_service &io_service, int fd) :
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
     *  The boost asio io_service.
     *  @var class boost::asio::io_service&
     */
    boost::asio::io_service & _ioservice;


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
    void monitor(TcpConnection *connection, int fd, int flags) override
    {
        // do we already have this filedescriptor
        auto iter = _watchers.find(fd);

        // was it found?
        if (iter == _watchers.end())
        {
            // we did not yet have this watcher - but that is ok if no filedescriptor was registered
            if (flags == 0){ return; }

            // construct a new pair (watcher/timer), and put it in the map
            _watchers[fd] = std::make_shared<Watcher>(_ioservice, fd);
            _watchers[fd]->events(connection, fd, flags);
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


public:

    /**
     *  Handler cannot be default constructed.
     *
     *  @param  that    The object to not move or copy
     */
    LibBoostAsioHandler() = delete;

    /**
     *  Constructor
     *  @param  loop    The boost io_service to wrap
     */
    explicit LibBoostAsioHandler(boost::asio::io_service &io_service) : _ioservice(io_service) {}

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

