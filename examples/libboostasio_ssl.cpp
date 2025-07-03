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
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/ssl.hpp>

#include <amqpcpp.h>
#include <amqpcpp/libboostasio.h>

/**
 *  Main program
 *  @return int
 */
int main()
{
    boost::asio::io_context io_context;

    boost::asio::signal_set signal_set{io_context, SIGINT, SIGTERM};

    signal_set.async_wait([&io_context] (const boost::system::error_code& error, int signal_number) {
        std::cerr << "Got signal " << signal_number << ", terminating..." << std::endl;

        io_context.stop();
    });

    const AMQP::Address address("amqps://guest:guest@localhost/");

    boost::asio::ip::tcp::resolver resolver(io_context);
    boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(address.hostname(), address.secure() ? "amqps" : "amqp");

    boost::asio::ssl::context ssl_context(boost::asio::ssl::context::sslv23);
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket(io_context, ssl_context);
    boost::asio::connect(ssl_socket.lowest_layer(), endpoints);

    ssl_socket.set_verify_mode(boost::asio::ssl::verify_none);
    ssl_socket.handshake(ssl_socket.client);

    // make a connection
    AMQP::LibBoostAsioConnection connection(std::move(ssl_socket), address.login(), address.vhost());

    // we need a channel too
    AMQP::LibBoostAsioChannel channel(&connection);

    // create a temporary queue
    channel.declareQueue(AMQP::exclusive).onSuccess([&connection](const std::string &name, uint32_t messagecount, uint32_t consumercount) {

        // report the name of the temporary queue
        std::cout << "declared queue " << name << std::endl;

        // now we can close the connection
        connection.close();
    });

    // run the handler
    // at the moment, one will need SIGINT to stop.  In time, should add signal handling through boost API.
    return io_context.run();
}

