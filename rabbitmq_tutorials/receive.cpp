#include <iostream>

#include "asiohandler.h"

int main(void)
{
    boost::asio::io_service ioService;
    AsioHandler handler(ioService);
    handler.connect("localhost", 5672);

    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");

    AMQP::Channel channel(&connection);
    channel.declareQueue("hello");
    channel.consume("hello", AMQP::noack).onReceived(
            [](const AMQP::Message &message,
                       uint64_t deliveryTag,
                       bool redelivered)
            {

                std::cout <<" [x] Received "<<message.message() << std::endl;
            });

    std::cout << " [*] Waiting for messages. To exit press CTRL-C\n";

    ioService.run();
    return 0;
}
