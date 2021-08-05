#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <iostream>
#include <optional>
#include <random>

#define TRACE std::cerr << __PRETTY_FUNCTION__ << " line " << __LINE__ << '\n'

constexpr const char *gQueueName = "my_test_queue";

std::vector<char> makeRandomMessage(const size_t size)
{
    std::vector<char> result;
    result.reserve(size);
    std::generate_n(std::back_inserter(result), size, []()
    {
        static std::random_device seed;
        static std::mt19937 engine(seed());
        static std::uniform_int_distribution<char> distribution('A', 'z');
        return distribution(engine);
    });
    return result;
}

struct MyHandler final : public AMQP::LibEvHandler, public std::enable_shared_from_this<MyHandler>
{
    struct ev_loop *loop;
    AMQP::TcpConnection *conn = nullptr;
    std::optional<AMQP::TcpChannel> channel;
    ev_idle idle;
    size_t messageCount;
    size_t messageSize;
    size_t current = 0;

    MyHandler(struct ev_loop *loop, size_t messageCount, size_t messageSize) :
        AMQP::LibEvHandler(loop),
        loop(loop),
        messageCount(messageCount),
        messageSize(messageSize)
    {
        ev_idle_init(&idle, idlecallback);
        idle.data = this;
    }

    ~MyHandler() override
    {
        ev_idle_stop(loop, &idle);
    }

    void onAttached(AMQP::TcpConnection *connection) override
    {
        AMQP::LibEvHandler::onAttached(connection);
        TRACE;
    }
    void onConnected(AMQP::TcpConnection *connection) override
    {
        AMQP::LibEvHandler::onConnected(connection);
        TRACE;
    }
    bool onSecuring(AMQP::TcpConnection *connection, SSL *ssl) override
    {
        TRACE;
        return AMQP::LibEvHandler::onSecuring(connection, ssl);
    }
    bool onSecured(AMQP::TcpConnection *connection, const SSL *ssl) override
    {
        TRACE;
        return AMQP::LibEvHandler::onSecured(connection, ssl);
    }
    void onProperties(AMQP::TcpConnection *connection, const AMQP::Table &server, AMQP::Table &client) override
    {
        AMQP::LibEvHandler::onProperties(connection, server, client);
        TRACE;
    }
    uint16_t onNegotiate(AMQP::TcpConnection *connection, uint16_t interval) override
    {
        TRACE;
        return AMQP::LibEvHandler::onNegotiate(connection, interval);
    }
    void onReady(AMQP::TcpConnection *connection) override
    {
        AMQP::LibEvHandler::onReady(connection);
        TRACE;

        conn = connection;
        channel.emplace(conn);
        channel->onError([weakself = weak_from_this()](const char *message)
        {
            if (auto self = weakself.lock()) self->conn->close();
        });
        std::cerr << "publishing " << messageCount << " messages each of size " << messageSize << " bytes to " << gQueueName << " ...\n";
        ev_idle_start(loop, &idle);
    }
    // void onHeartbeat(AMQP::TcpConnection *connection) override
    // {
    //     AMQP::LibEvHandler::onHeartbeat(connection);
    //     TRACE;
    // }
    void onError(AMQP::TcpConnection *connection, const char *message) override
    {
        AMQP::LibEvHandler::onError(connection, message);
        TRACE;
    }
    void onClosed(AMQP::TcpConnection *connection) override
    {
        AMQP::LibEvHandler::onClosed(connection);
        TRACE;
    }
    void onLost(AMQP::TcpConnection *connection) override
    {
        AMQP::LibEvHandler::onLost(connection);
        TRACE;
    }
    void onDetached(AMQP::TcpConnection *connection) override
    {
        AMQP::LibEvHandler::onDetached(connection);
        TRACE;
    }

    void close()
    {
        std::cerr << "closing channel\n";
        channel->close().onFinalize([self = shared_from_this()]()
        {
            std::cerr << "channel closed\n";
            std::cerr << "closing connection\n";
            self->conn->close();
        });
    }

    void onIdle()
    {
        ev_idle_stop(loop, &idle);
        for (size_t i = 0; i != 256; ++i, ++current)
        {
            if (current == messageCount)
            {
                channel->close().onFinalize([self = shared_from_this()]()
                {
                    std::cerr << "done publishing messages, closing connection\n";
                    self->conn->close();
                });
                return;
            }
            const auto msg = makeRandomMessage(messageSize);
            channel->publish("", gQueueName, msg.data(), msg.size());
        }
        ev_idle_start(loop, &idle);
    }

    static void idlecallback(struct ev_loop *loop, ev_idle *idle, int flags)
    {
        static_cast<MyHandler*>(idle->data)->onIdle();
    }
};

int main(const int argc, const char **argv)
{
    const AMQP::Address address(argv[1]);
    auto *loop = ev_default_loop();
    auto handler = std::make_shared<MyHandler>(loop, std::atoi(argv[2]), std::atoi(argv[3]));
    AMQP::TcpConnection connection(handler.get(), address);
    ev_run(loop);
    return EXIT_SUCCESS;
}
