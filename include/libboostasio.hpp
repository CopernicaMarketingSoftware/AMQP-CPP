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
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 *  @note Because of a limitation with boost::asio on Windows, this will only work on POSIX based systems - see https://github.com/chriskohlhoff/asio/issues/70
 */
class LibBoostAsioHandler : public virtual TcpHandler
{
private:

    /**
     *  Helper class that wraps a boost io_service socket monitor.
     */
    class Watcher
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
        bool    _read{false};

        /**
         *  A boolean that indicates if the watcher is monitoring for write events.
         *  @var _read True if writes are being monitored else false.
         */
        bool    _write{false};

        /**
         *  Handler method that is called by boost's io_service when the socket pumps a read event.
         *  @param  ec          The status of the callback.
         *  @param  connection  The connection being watched.
         *  @param  fd          The file descriptor being watched.
         *  @note   The handler will get called if a read is cancelled.
         */
        void read_handler(boost::system::error_code ec, TcpConnection *connection, int fd)
        {
            if (!ec && _read)
            {
                connection->process(fd, AMQP::readable);

                _socket.async_read_some(boost::asio::null_buffers(),
                                        boost::bind(&Watcher::read_handler,
                                                    this,
                                                    boost::placeholders::_1,
                                                    connection,
                                                    fd));
            }
        }

        /**
         *  Handler method that is called by boost's io_service when the socket pumps a write event.
         *  @param  ec          The status of the callback.
         *  @param  connection  The connection being watched.
         *  @param  fd          The file descriptor being watched.
         *  @note   The handler will get called if a write is cancelled.
         */
        void write_handler(boost::system::error_code ec, TcpConnection *connection, int fd)
        {
            if (!ec && _write)
            {
                connection->process(fd, AMQP::writable);

                _socket.async_write_some(boost::asio::null_buffers(),
                                         boost::bind(&Watcher::write_handler,
                                                     this,
                                                     boost::placeholders::_1,
                                                     connection,
                                                     fd));
            }
        }

    public:
        /**
         *  Constructor
         *  @param  io_service      The boost io_service
         *  @param  connection      The connection being watched
         *  @param  fd              The filedescriptor being watched
         *  @param  events          The events that should be monitored
         */
        Watcher(boost::asio::io_service &io_service, TcpConnection *connection, int fd, int events) :
            _ioservice(io_service),
            _socket(_ioservice)
        {
            _socket.assign(fd);

            _socket.non_blocking(true);

            // configure monitoring
            this->events(connection,fd,events);
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
            _socket.release();
        }

        /**
         *  Change the events for which the filedescriptor is monitored
         *  @param  events
         */
        void events(TcpConnection *connection, int fd, int events)
        {
            bool bRead(_read);
            bool bWrite(_write);

            // 1. Handle reads?
            _read = ((events & AMQP::readable) != 0);

            if (!bRead && _read)
            {
                _socket.async_read_some(boost::asio::null_buffers(),
                                        boost::bind(&Watcher::read_handler,
                                                    this,
                                                    boost::placeholders::_1,
                                                    connection,
                                                    fd));
            }

            // 2. Handle writes?
            _write = ((events & AMQP::writable) != 0);

            if (!bWrite && _write)
            {
                _socket.async_write_some(boost::asio::null_buffers(),
                                         boost::bind(&Watcher::write_handler,
                                                     this,
                                                     boost::placeholders::_1,
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
    std::map<int, std::unique_ptr<Watcher> > _watchers;


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
            _watchers[fd] = std::make_unique<Watcher>(_ioservice, connection, fd, flags);
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
     *  Destructor
     */
    ~LibBoostAsioHandler() override = default;
};


/**
 *  End of namespace
 */
}

