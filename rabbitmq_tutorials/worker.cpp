#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>

#include "asiohandler.h"

int main(void)
{
    boost::asio::io_service ioService;
    AsioHandler handler(ioService);
    handler.connect("localhost", 5672);

    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");

    AMQP::Channel channel(&connection);
    channel.setQos(1);

    channel.declareQueue("task_queue", AMQP::durable);
    channel.consume("task_queue").onReceived(
            [&channel](const AMQP::Message &message,
                       uint64_t deliveryTag,
                       bool redelivered)
            {
                const auto body = message.message();
                std::cout<<" [x] Received "<<body<<std::endl;

                size_t count = std::count(body.cbegin(), body.cend(), '.');
                std::this_thread::sleep_for (std::chrono::seconds(count));

                std::cout<<" [x] Done"<<std::endl;
                channel.ack(deliveryTag);
            });


    std::cout << " [*] Waiting for messages. To exit press CTRL-C\n";

    ioService.run();
    return 0;
}
