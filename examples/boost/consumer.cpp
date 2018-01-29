/**
 *  LibBoostAsio.cpp
 * 
 *  Test program to check AMQP functionality based on Boost's asio io_service.
 * 
 *  @author Gavin Smith <gavin.smith@coralbay.tv>
 *
 *  Compile with g++ -std=c++14 libboostasio.cpp -o boost_test -lpthread -lboost_system -lamqpcpp
 */

/**
 *  Dependencies
 */

#include <memory> // std::shared_ptr

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/deadline_timer.hpp>


#include <amqpcpp.h>
#include <amqpcpp/libboostasio.h>


// callback function that is called when the consume operation starts
auto startCb = [](const std::string &consumertag) {

    std::cout << "consume operation started" << std::endl;
};

// callback function that is called when the consume operation failed
auto errorCb = [](const char *message) {

    std::cout << "consume operation failed" << std::endl;
};


void printRabbitMessage(const AMQP::Message &message)
{
    std::string message_str(message.body(), message.bodySize());
    std::cout << "[" << message.exchange() << "] : " << message_str << std::endl;
    if (message.hasCorrelationID())
	    std::cout << "Correlation ID: " << message.correlationID() << std::endl;
    if (message.hasReplyTo())
	    std::cout << "Replay to : " << message.replyTo() << std::endl;


}

// callback operation when a message was received
auto messageCb = [](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {

	printRabbitMessage(message);
};


/**
 *  Main program
 *  @return int
 */
int dynamic_main()
{

    // access to the boost asio handler
    // note: we suggest use of 2 threads - normally one is fin (we are simply demonstrating thread safety).
    boost::asio::io_service service(4);

    // create a work object to process our events.
    boost::asio::io_service::work work(service);
    
    // handler for libev
    AMQP::LibBoostAsioHandler handler(service);
    
    // make a connection
    //
    std::shared_ptr <AMQP::TcpConnection> connection;
    connection = std::make_shared<AMQP::TcpConnection>(&handler, AMQP::Address("amqp://guest:guest@localhost/"));
    std::cout << "Create connection to amqp://guest:guest@localhost/\n";

    // we need a channel too
    AMQP::TcpChannel channel(connection.get());
    std::cout << "Create channel\n";

    channel.declareQueue("my_event_queue").onSuccess([&channel](const std::string& name, uint32_t messageCount, uint32_t consumerCount) {
   
	  std::cout << "Queue '" << name << "' has been declared with " << messageCount << " messages and " << consumerCount << " consumers" << std::endl;
	  channel.bindQueue("EVENT_EXCHANGE", "my_event_queue", "event.am.*").onError([&name](const char* message){
	  	std::cout << "Binding " << name << " FAILED\n";
		std::cout << "Message: " << message << std::endl;
	  }).onSuccess([&channel](){

    	  channel.consume("my_event_queue")
    		.onReceived(messageCb)
		.onSuccess(startCb)
		.onError(errorCb);
	});
 
    });
 
  
    // run the handler
    // a t the moment, one will need SIGINT to stop.  In time, should add signal handling through boost API.
    std::cout << "Start example...\n";
    return service.run();
}


int main()
{
	return dynamic_main();
	// return static_main();

}
