/**
 *  LibBoostAsio.cpp
 * 
 *  Test program to check AMQP functionality based on Boost's asio io_service.
 * 
 *  @author Alessandro Pischedda <alessandro.pischedda@gmail.com>
 *
 *  Compile with g++ -std=c++11 libboostasio.cpp -o boost_test -lpthread -lboost_system -lamqpcpp
 */

#include <unistd.h>
#include <memory> // std::shared_ptr

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/program_options.hpp>

#include <amqpcpp.h>
#include <amqpcpp/libboostasio.h>


struct Options {
    std::string exchange_name;
    std::string exchange_type;
    std::string queue_name;
    std::string rabbit_server;
};


void usage()
{
    std::cout << "Producer Help" ;
}

bool handle_options(struct Options &opt, int argc, char* argv[])
{
    char c;

    while((c=getopt(argc, argv, "n:t:q:s:h")) != -1) {

        switch(c) {
                case 'n': 
                          opt->exchange_name = std::string(optarg);
                          break;

                case 't':
                          opt->exchange_type = std::string(optarg);
                          break;

                case 's':
                          opt->rabbit_server = std::string(optarg);
                          break;

                case 'q':
                          opt->queue_name = std::string(optarg);
                          break;

                case 'h': usage();
                          return false;
        } // END SWITCH
    } // END WHILE

    return true;
}

void run(Options opt)
{

    boost::asio::io_service service();

    // create a work object to process our events.
    boost::asio::io_service::work work(service);
    
    // handler for libboost
    AMQP::LibBoostAsioHandler handler(service);
    
    // make a connection
    AMQP::TcpConnection connection(&handler, AMQP::Address(opt.rabbit_server));

    AMQP::TcpChannel channel(&connection);

    // create the queue
    channel.declareQueue(queue_name).onSuccess([&connection](const std::string &name, uint32_t messagecount, uint32_t consumercount) {
        
        // report the name of the temporary queue
        std::cout << "declared queue " << name << std::endl;
 
        channel.declareExchange(exchange_name).onError([](const char* message)
        {
		    std::cout << "Error: " << message << std::endl;
	    }).onSuccess([&channel, &connection]()
        {
            std::string producer_message; 
            for(int i = 0; ; i++)
            {
                producer_message = "Message " + std::to_string(i);
	 	        channel.publish(exchange_name, "", producer_message);
            }
	    });
       
    });
    
    // run the handler
    // a t the moment, one will need SIGINT to stop.  In time, should add signal handling through boost API.
    std::cout << "Start producer...\n";
    service.run();
}



int main(int argc, char* argv[])
{
    Options opt;
    if (!handle_options(opt, argc, argv)
            return -1;
    run(opt);
}
