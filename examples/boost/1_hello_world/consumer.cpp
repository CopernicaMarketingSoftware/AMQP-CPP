/**
 *  consumer.cpp
 *
 *  @author Alessandro Pischedda <alessandro.pischedda@gmail.com>
 *
 *  Compile with g++ -std=c++11 consumer.cpp -o consumer -lpthread -lboost_system -lamqpcpp
 */

#include <unistd.h>

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/deadline_timer.hpp>

#include <amqpcpp.h>
#include <amqpcpp/libboostasio.h>


// callback operation when a message was received
auto messageCb = [](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {

    std::string message_str(message.body(), message.bodySize());
    std::cout << "[x] Received '" << message_str << "'" << std::endl;
};


int main(int argc, char* argv[])
{
    struct Data {
            std::string queue_name;
            std::string message;
            std::string routing_key;
            std::string exchange_name;
    };

    Data  data;

    data.queue_name    = "hello";
    data.message       = "Hello World!";
    data.routing_key   = "hello";
    data.exchange_name = "";

    boost::asio::io_service service;

    // create a work object to process our events.
    boost::asio::io_service::work work(service);
    
    // handler for libboost
    AMQP::LibBoostAsioHandler handler(service);
    
    // make a connection
    AMQP::TcpConnection connection(&handler, AMQP::Address("amqp://guest:guest@localhost/"));

    AMQP::TcpChannel channel(&connection);

    // create the queue
    channel.declareQueue(data.queue_name).onSuccess(
                [&channel, &data](const std::string &name, uint32_t messagecount, uint32_t consumercount)
    {
    	channel.consume(data.queue_name).onReceived(messageCb);
    }); 

    std::cout << "[*] Waiting for messages. To exit press CTRL+C" << std::endl;
    service.run();

    return 0;
}


