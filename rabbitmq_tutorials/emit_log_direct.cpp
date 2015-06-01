#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "tools.h"
#include "asiohandler.h"

int main(int argc, const char* argv[])
{
    const std::string severity = argc > 2 ? argv[1] : "info";
    const std::string msg =
            argc > 2 ? join(&argv[2], &argv[argc], " ") : "Hello World!";

    boost::asio::io_service ioService;
    AsioHandler handler(ioService);
    handler.connect("localhost", 5672);

    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");

    boost::asio::deadline_timer t(ioService, boost::posix_time::millisec(100));
    AMQP::Channel channel(&connection);
    channel.declareExchange("direct_logs", AMQP::direct).onSuccess([&]()
    {
        channel.publish("direct_logs", severity, msg);
        std::cout << " [x] Sent "<<severity<<":"<<msg<< std::endl;

        t.async_wait([&](const boost::system::error_code&){ioService.stop();});
    });

    ioService.run();
    return 0;
}
