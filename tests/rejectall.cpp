#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <iostream>
#include <optional>

#define TRACE std::cerr << __PRETTY_FUNCTION__ << " line " << __LINE__ << '\n'

struct MyHandler final : public AMQP::LibEvHandler, public std::enable_shared_from_this<MyHandler>
{
    struct ev_loop *loop;
    ev_timer *timer;
    uint16_t prefetchCount;
    const char *queueName;
    AMQP::TcpConnection *conn = nullptr;
    std::optional<AMQP::TcpChannel> channel;
    std::string consumertag;

    MyHandler(struct ev_loop *loop, ev_timer *timer, uint16_t prefetchCount, const char *queueName) :
        AMQP::LibEvHandler(loop),
        loop(loop),
        timer(timer),
        prefetchCount(prefetchCount),
        queueName(queueName)
    {
    }

    ~MyHandler() override = default;

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
        channel->setQos(prefetchCount);
        channel->consume(queueName)
            .onReceived([self = shared_from_this()](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
            {
                self->channel->reject(deliveryTag);
            })
            .onSuccess([self = shared_from_this()](const std::string &tag)
            {
                std::cerr << "consuming from queue \"" << self->queueName << "\" now (tag: " << tag << ")\n";
                self->consumertag = tag;
            })
            .onError([self = shared_from_this()](const char *message)
            {
                std::cerr << "failed to consume from queue \"" << self->queueName << "\": " << message << '\n';
            });
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

    void onTimeout()
    {
        TRACE;
        if (!channel) return;

        AMQP::Table properties;
        properties["x-dead-letter-exchange"] = "";
        properties["x-dead-letter-routing-key"] = "in";
        properties["x-queue-mode"] = "lazy";

        channel->declareQueue(queueName, AMQP::durable, properties)
            .onSuccess([self = shared_from_this()](const std::string &name, uint32_t messagecount, uint32_t consumercount)
            {
                if (self->channel && messagecount == 0)
                {
                    std::cerr << "cancelling consumer\n";
                    self->cancel();
                }
            })
            .onError([self = shared_from_this()](const char *message)
            {
                std::cerr << "error declaring queue: " << message << '\n';
                self->cancel();
            });
    }

    void cancel()
    {
        std::cerr << "stopping timer\n";
        ev_timer_stop(loop, timer);
        std::cerr << "cancelling consumer\n";
        if (consumertag.empty())
        {
            close();
        }
        else
        {
            channel->cancel(queueName).onFinalize([self = shared_from_this()]()
            {
                std::cerr << "consumer cancelled\n";
                self->close();
            });
        }
    }

    void close()
    {
        std::cerr << "closing channel\n";
        channel->close().onFinalize([self = shared_from_this()]()
        {
            self->channel.reset();
            std::cerr << "channel closed\n";
            std::cerr << "closing connection\n";
            self->conn->close();
        });
    }

    static void timercallback(struct ev_loop *loop, ev_timer *timer, int flags)
    {
        static_cast<MyHandler*>(timer->data)->onTimeout();
    }

};

int main(const int argc, const char **argv)
{
    OPENSSL_init_ssl(0, nullptr);
    const AMQP::Address address(argv[1]);
    auto *loop = ev_default_loop();
    ev_timer timer;
    ev_timer_init(&timer, MyHandler::timercallback, 5.0, 5.0);
    auto handler = std::make_shared<MyHandler>(loop, &timer, static_cast<uint16_t>(std::atoi(argv[2])), argv[3]);
    timer.data = handler.get();
    ev_timer_start(loop, &timer);
    AMQP::TcpConnection connection(handler.get(), address);
    ev_run(loop);
    return EXIT_SUCCESS;
}
