#include <iostream>
#include <algorithm>

#include "asiohandler.h"

int main(int argc, const char* argv[])
{
    if(argc==1)
    {
        std::cout<<"Usage: "<<argv[0]<<" [binding_key]..."<<std::endl;
        return 1;
    }

    boost::asio::io_service ioService;
    AsioHandler handler(ioService);
    handler.connect("localhost", 5672);

    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");

    AMQP::Channel channel(&connection);

    channel.declareExchange("topic_logs", AMQP::topic);

    auto receiveMessageCallback =
            [](const AMQP::Message &message,
               uint64_t deliveryTag,
               bool redelivered)
            {
                std::cout <<" [x] "
                          <<message.routingKey()
                          <<":"
                          <<message.message()
                          << std::endl;
            };

    AMQP::QueueCallback callback = [&](const std::string &name,
            int msgcount,
            int consumercount)
    {
        std::for_each(&argv[1],
                &argv[argc],
                [&](const char* bindingKeys)
                {
                    std::cout<<bindingKeys<<std::endl;
                    channel.bindQueue("topic_logs",name, bindingKeys);
                    channel.consume(name, AMQP::noack).onReceived(receiveMessageCallback);
                });

    };
    channel.declareQueue(AMQP::exclusive).onSuccess(callback);

    std::cout << " [*] Waiting for messages. To exit press CTRL-C\n";
    ioService.run();
    return 0;
}
