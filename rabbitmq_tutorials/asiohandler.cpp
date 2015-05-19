#include <iostream>

#include "asiohandler.h"

using boost::asio::ip::tcp;

class AmqpBuffer
{
public:
    AmqpBuffer(size_t size) :
            _data(size, 0),
            _use(0)
    {
    }

    size_t write(const char* data, size_t size)
    {
        if (_use == _data.size())
        {
            return 0;
        }

        const size_t length = (size + _use);
        size_t write = length < _data.size() ? size : _data.size() - _use;
        memcpy(_data.data() + _use, data, write);
        _use += write;
        return write;
    }

    void drain()
    {
        _use = 0;
    }

    size_t available() const
    {
        return _use;
    }

    const char* data() const
    {
        return _data.data();
    }

    void shl(size_t count)
    {
        assert(count < _use);

        const size_t diff = _use - count;
        std::memmove(_data.data(), _data.data() + count, diff);
        _use = _use - count;
    }

private:
    std::vector<char> _data;
    size_t _use;
};

AsioHandler::AsioHandler(boost::asio::io_service& ioService) :
        _ioService(ioService),
        _socket(ioService),
        _timer(ioService),
        _asioInputBuffer(ASIO_INPUT_BUFFER_SIZE, 0),
        _amqpBuffer(new AmqpBuffer(ASIO_INPUT_BUFFER_SIZE * 2)),
        _connection(nullptr),
        _writeInProgress(false),
        _connected(false),
        _quit(false)
{
}

AsioHandler::~AsioHandler()
{
}

void AsioHandler::connect(const std::string& host, uint16_t port)
{
    doConnect(host, port);
}

void AsioHandler::doConnect(const std::string& host, uint16_t port)
{
    tcp::resolver::query query(host, std::to_string(port));
    tcp::resolver::iterator iter = tcp::resolver(_ioService).resolve(query);
    _timer.expires_from_now(boost::posix_time::seconds(15));
    _timer.async_wait([this](const boost::system::error_code& ec){
        if(!ec && !_connected)
        {
            std::cerr<<"Connection timed out";
            _socket.cancel();
            exit(1);
        }
    });

    boost::asio::async_connect(_socket, iter,
            [this](boost::system::error_code ec, tcp::resolver::iterator)
            {
                _connected = true;
                if (!ec)
                {
                    doRead();

                    if(!_outputBuffer.empty())
                    {
                        doWrite();
                    }
                }
                else
                {
                    std::cerr<<"Connection error:"<<ec<<std::endl;
                    exit(1);
                }
            });

}

void AsioHandler::onData(
        AMQP::Connection *connection, const char *data, size_t size)
{
    _connection = connection;

    _outputBuffer.push_back(std::vector<char>(data, data + size));
    if (!_writeInProgress && _connected)
    {
        doWrite();
    }
}

void AsioHandler::doRead()
{
    _socket.async_receive(boost::asio::buffer(_asioInputBuffer),
            [this](boost::system::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    _amqpBuffer->write(_asioInputBuffer.data(), length);
                    parseData();
                    doRead();
                }
                else
                {
                    std::cerr<<"Error reading:"<<ec<<std::endl;
                    exit(1);
                }
            });
}

void AsioHandler::doWrite()
{
    _writeInProgress = true;
    boost::asio::async_write(_socket,
            boost::asio::buffer(_outputBuffer.front()),
            [this](boost::system::error_code ec, std::size_t length )
            {
                if(!ec)
                {
                    _outputBuffer.pop_front();
                    if(!_outputBuffer.empty())
                    {
                        doWrite();
                    }
                    else
                    {
                        _writeInProgress = false;
                    }

                    if(_quit)
                    {
                        _socket.close();
                    }
                }
                else
                {
                    std::cerr<<"Error writing:"<<ec<<std::endl;
                    _socket.close();
                    exit(1);
                }
            });
}

void AsioHandler::parseData()
{
    if (_connection == nullptr)
    {
        return;
    }

    const size_t count = _connection->parse(_amqpBuffer->data(),
            _amqpBuffer->available());

    if (count == _amqpBuffer->available())
    {
        _amqpBuffer->drain();
    }
    else if (count > 0)
    {
        _amqpBuffer->shl(count);
    }
}

void AsioHandler::onConnected(AMQP::Connection *connection)
{
}
bool AsioHandler::connected() const
{
    return _connected;
}

void AsioHandler::onError(AMQP::Connection *connection, const char *message)
{
    std::cerr << "AMQP error " << message << std::endl;
}

void AsioHandler::onClosed(AMQP::Connection *connection)
{
    std::cout << "AMQP closed connection" << std::endl;
    _quit = true;
    if (!_writeInProgress)
    {
        _socket.close();
    }
}
