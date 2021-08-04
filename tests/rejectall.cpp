#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <iostream>
#include <optional>

#define TRACE std::cerr << __PRETTY_FUNCTION__ << " line " << __LINE__ << '\n'

struct MyHandler final : public AMQP::LibEvHandler, public std::enable_shared_from_this<MyHandler>
{
    struct ev_loop *loop;
    AMQP::TcpConnection *conn = nullptr;
    std::optional<AMQP::TcpChannel> channel;
    MyHandler(struct ev_loop *loop) : AMQP::LibEvHandler(loop), loop(loop) {}
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
        channel->consume("my_test_queue")
            .onReceived([self = shared_from_this()](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
            {
                std::cerr << "got message with delivery tag " << deliveryTag << '\n';

                // message body
                message.body();

                // message size
                message.size();

                // self->channel->ack or self->channel->reject the deliveryTag
                self->channel->reject(deliveryTag);
            })
            .onSuccess([self = shared_from_this()]()
            {
                std::cerr << "consuming from queue now\n";
            })
            .onError([self = shared_from_this()](const char *message)
            {
                std::cerr << "failed to consume from queue: " << message << '\n';
            });
    }
    void onHeartbeat(AMQP::TcpConnection *connection) override
    {
        AMQP::LibEvHandler::onHeartbeat(connection);
        TRACE;
    }
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

};

int main(const int argc, const char **argv)
{
    const AMQP::Address address(argv[1]);
    auto *loop = ev_default_loop();
    auto handler = std::make_shared<MyHandler>(loop);
    AMQP::TcpConnection connection(handler.get(), address);
    ev_run(loop);
    return EXIT_SUCCESS;
}
