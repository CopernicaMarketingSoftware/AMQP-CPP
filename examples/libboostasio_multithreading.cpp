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

#include <array>
#include <chrono>
#include <thread>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/signal_set.hpp>

#include <amqpcpp.h>
#include <amqpcpp/libboostasio.h>


/**
 *  Main program
 *  @return int
 */
int main()
{
    using namespace std::chrono_literals;

    boost::asio::io_context io_context;

    boost::asio::signal_set signal_set{io_context, SIGINT, SIGTERM};

    signal_set.async_wait([&io_context] (const boost::system::error_code& error, int signal_number) {
        std::cerr << "Got signal " << signal_number << ", terminating..." << std::endl;

        io_context.stop();
    });

    boost::asio::strand<boost::asio::io_context::executor_type> strand(io_context.get_executor());

    const AMQP::Address address("amqp://guest:guest@localhost/");

    boost::asio::ip::tcp::resolver resolver(io_context);
    boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(address.hostname(), address.secure() ? "amqps" : "amqp");

    boost::asio::ip::tcp::socket socket(strand);
    boost::asio::connect(socket, endpoints);

    // make a connection
    AMQP::LibBoostAsioConnection connection(std::move(socket), address.login(), address.vhost());

    std::array<AMQP::LibBoostAsioChannel, 4> channels {{
        AMQP::LibBoostAsioChannel{&connection},
        AMQP::LibBoostAsioChannel{&connection},
        AMQP::LibBoostAsioChannel{&connection},
        AMQP::LibBoostAsioChannel{&connection}
    }};

    // create a temporary queue
    channels[0].declareQueue(AMQP::exclusive).onSuccess([&io_context, &strand, &connection, &channels](const std::string &name, uint32_t messagecount, uint32_t consumercount) {
        // report the name of the temporary queue
        std::cout << "declared queue " << name << std::endl;

        // fill the queue
        for (std::size_t i = 0; i < 100; ++i) {
            channels[0].publish("", name, std::to_string(i).c_str());
        }

        for (auto& channel: channels) {
            channel.setQos(1).onSuccess([&io_context, &strand, &channel, name] () {
                channel.consume(name).onReceived([&io_context, &strand, &channel] (const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {
                    std::cout << "delivery tag " << deliveryTag << " by " << channel.id() << " body " << std::string(message.body(), message.bodySize()) << std::endl;

                    boost::asio::post(io_context, [&channel, &strand, deliveryTag] () {
                        std::this_thread::sleep_for(1s);

                        boost::asio::post(strand, [&channel, deliveryTag] () {
                            channel.ack(deliveryTag);

                            std::cout << "ack " << deliveryTag << " by " << channel.id() << std::endl;
                        });
                    });
                });
            });
        }
    });

    const auto tf = [&io_context] () {
            io_context.run();
    };
    std::array<std::thread, 4> pool {{
        std::thread{tf},
        std::thread{tf},
        std::thread{tf},
        std::thread{tf}
    }};

    for (auto& thread: pool) {
        thread.join();
    }

    return 0;
}

