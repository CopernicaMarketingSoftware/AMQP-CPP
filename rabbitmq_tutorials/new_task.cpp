#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "tools.h"
#include "asiohandler.h"

int main(int argc, const char* argv[])
{
    const std::string msg =
            argc > 1 ? join(&argv[1], &argv[argc], " ") : "Hello World!";

    boost::asio::io_service ioService;
    AsioHandler handler(ioService);
    handler.connect("localhost", 5672);

    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");
    AMQP::Channel channel(&connection);

    boost::asio::deadline_timer t(ioService, boost::posix_time::millisec(100));
    AMQP::QueueCallback callback =
            [&](const std::string &name, int msgcount, int consumercount)
            {
                AMQP::Envelope env(msg);
                env.setDeliveryMode(2);
                channel.publish("", "task_queue", env);
                std::cout<<" [x] Sent '"<<msg<<"'\n";

                t.async_wait([&](const boost::system::error_code&){ioService.stop();});
            };

    channel.declareQueue("task_queue", AMQP::durable).onSuccess(callback);

    ioService.run();
    return 0;
}
