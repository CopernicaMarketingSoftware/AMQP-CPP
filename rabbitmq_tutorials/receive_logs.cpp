#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "asiohandler.h"

int main(void)
{
    boost::asio::io_service ioService;
    AsioHandler handler(ioService);
    handler.connect("localhost", 5672);

    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");

    AMQP::Channel channel(&connection);
    auto receiveMessageCallback = [](const AMQP::Message &message,
            uint64_t deliveryTag,
            bool redelivered)
    {

        std::cout <<" [x] "<<message.message() << std::endl;
    };

    AMQP::QueueCallback callback =
            [&](const std::string &name, int msgcount, int consumercount)
            {
                channel.bindQueue("logs", name,"");
                channel.consume(name, AMQP::noack).onReceived(receiveMessageCallback);
            };

    AMQP::SuccessCallback success = [&]()
            {
                channel.declareQueue(AMQP::exclusive).onSuccess(callback);
            };

    channel.declareExchange("logs", AMQP::fanout).onSuccess(success);

    std::cout << " [*] Waiting for messages. To exit press CTRL-C\n";

    ioService.run();
    return 0;
}
