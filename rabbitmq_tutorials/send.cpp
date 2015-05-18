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

    boost::asio::deadline_timer t(ioService, boost::posix_time::millisec(100));
    channel.onReady([&]()
    {
        if(handler.connected())
        {
            channel.publish("", "hello", "Hello World!");
            std::cout << " [x] Sent 'Hello World!'" << std::endl;

            t.async_wait([&](const boost::system::error_code&){ioService.stop();});
        }
    });

    ioService.run();
    return 0;
}
