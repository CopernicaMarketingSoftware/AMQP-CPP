/**
 *  producer.cpp
 *
 *  @author Alessandro Pischedda <alessandro.pischedda@gmail.com>
 *
 *  Compile with g++ -std=c++11 producer.cpp -o producer -lpthread -lboost_system -lamqpcpp
 */

#include <unistd.h>

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/deadline_timer.hpp>

#include <amqpcpp.h>
#include <amqpcpp/libboostasio.h>

int main(int argc, char* argv[])
{

    struct Data {
        std::string queue_name;
        std::string message;
        std::string routing_key;
        std::string exchange_name;
    };

    Data data;
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
                [&connection, &service, &channel, &data](const std::string &name, uint32_t messagecount, uint32_t consumercount)
    {
        
	    channel.publish(data.exchange_name, data.routing_key, data.message);
        std::cout << "[x] Sent " << data.message << std::endl;

        // close connection
        connection.close();
        // stop io_service
        service.stop();
    }); 
    std::cout << "Start producer...\n";
    service.run();

    return 0;
}

