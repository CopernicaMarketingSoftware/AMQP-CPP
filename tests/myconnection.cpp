/**
 *  MyConnection.cpp
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Required external libraries
 */
#include <amqpcpp.h>
#include <copernica/network.h>

#include <string>

/**
 *  Namespaces to use
 */
using namespace std;
using namespace Copernica;

/**
 *  Required local class definitions
 */
#include "myconnection.h"

/**
 *  Constructor
 */
MyConnection::MyConnection(const std::string &ip) :
    _socket(Event::MainLoop::instance(), this),
    _connection(nullptr),
    _channel(nullptr)
{
    // start connecting
    if (_socket.connect(Network::Ipv4Address(ip), 5672)) return;

    // failure
    onFailure(&_socket);
}

/**
 *  Method that is called when the connection failed
 *  @param  socket      Pointer to the socket
 */
void MyConnection::onFailure(Network::TcpSocket *socket)
{
    // report error
    std::cout << "connect failure" << std::endl;
}

/**
 *  Method that is called when the connection timed out (which also is a failure
 *  @param  socket      Pointer to the socket
 */
void MyConnection::onTimeout(Network::TcpSocket *socket)
{
    // report error
    std::cout << "connect timeout" << std::endl;
}

/**
 *  Method that is called when the connection succeeded
 *  @param  socket      Pointer to the socket
 */
void MyConnection::onConnected(Network::TcpSocket *socket)
{
    // report connection
    std::cout << "connected" << std::endl;

    // we are connected, leap out if there already is a amqp connection
    if (_connection) return;

    // create amqp connection, and a new channel
    _connection = std::unique_ptr<AMQP::Connection>(new AMQP::Connection(this, AMQP::Login("guest", "guest"), "/"));
    _channel = std::unique_ptr<AMQP::Channel>(new AMQP::Channel(_connection.get()));

    // watch for the channel becoming ready
    _channel->onReady([](AMQP::Channel *channel) {
        // show that we are ready
        std::cout << "AMQP channel ready, id: " << (int) channel->id() << std::endl;
    });

    // and of course for channel errors
    _channel->onError([this](AMQP::Channel *channel, const std::string& message) {
        // inform the user of the error
        std::cerr << "AMQP channel error on channel " << channel->id() << ": " << message << std::endl;

        // delete the channel
        _channel = nullptr;

        // close the connection
        _connection->close();
    });

    // declare a queue and let us know when it succeeds
    _channel->declareQueue("my_queue").onSuccess([](AMQP::Channel *channel, const std::string &name, uint32_t messageCount, uint32_t consumerCount){
        // queue was successfully declared
        std::cout << "AMQP Queue declared with name '" << name << "', " << messageCount << " messages and " << consumerCount << " consumer" << std::endl;
    });

    // also declare an exchange
    _channel->declareExchange("my_exchange", AMQP::direct).onSuccess([](AMQP::Channel *channel) {
        // exchange successfully declared
        std::cout << "AMQP exchange declared" << std::endl;
    });

    // bind the queue to the exchange
    _channel->bindQueue("my_exchange", "my_queue", "key").onSuccess([](AMQP::Channel *channel) {
        // queue successfully bound to exchange
        std::cout << "AMQP Queue bound" << std::endl;
    });

    // set quality of service
    _channel->setQos(1).onSuccess([](AMQP::Channel *channel) {
        // quality of service successfully set
        std::cout << "AMQP Quality of Service set" << std::endl;
    });

    // publish a message to the exchange
    if (!_channel->publish("my_exchange", "key", "my_message"))
    {
        // we could not publish the message, something is wrong somewhere
        std::cerr << "Unable to publish message" << std::endl;

        // close the channel
        _channel->close().onSuccess([this](AMQP::Channel *channel) {
            // also close the connection
            _connection->close();
        });
    }

    // consume the message we just published
    _channel->consume("my_queue", "my_consumer", AMQP::exclusive)
    .onReceived([this](AMQP::Channel *channel, const AMQP::Message &message, uint64_t deliveryTag, const std::string &consumerTag, bool redelivered) {
        // show the message data
        std::cout << "AMQP consumed: " << message.message() << std::endl;

        // ack the message
        _channel->ack(deliveryTag);

        // and stop consuming (there is only one message anyways)
        _channel->cancel("my_consumer").onSuccess([](AMQP::Channel *channel, const std::string& tag) {
            // we successfully stopped consuming
            std::cout << "Stopped consuming under tag " << tag << std::endl;
        });

        // unbind the queue again
        _channel->unbindQueue("my_exchange", "my_queue", "key").onSuccess([](AMQP::Channel *channel) {
            // queueu successfully unbound
            std::cout << "Queue unbound" << std::endl;
        });

        // the queue should now be empty, so we can delete it
        _channel->removeQueue("my_queue").onSuccess([](AMQP::Channel *channel, uint32_t messageCount) {
            // queue was removed, it should have been empty, so messageCount should be 0
            if (messageCount) std::cerr << "Removed queue which should have been empty but contained " << messageCount << " messages" << std::endl;

            // no messages is the expected behavior
            else std::cout << "Queue removed" << std::endl;
        });

        // also remove the exchange
        _channel->removeExchange("my_exchange").onSuccess([](AMQP::Channel *channel) {
            // exchange was successfully removed
            std::cout << "Removed exchange" << std::endl;
        });

        // everything done, close the channel
        _channel->close().onSuccess([this](AMQP::Channel *channel) {
            // channel was closed
            std::cout << "Channel closed" << std::endl;

            // close the connection too
            _connection->close();
        });
    })
    .onSuccess([](AMQP::Channel *channel, const std::string& tag) {
        // consumer was started
        std::cout << "Started consuming under tag " << tag << std::endl;
    });
}

/**
 *  Method that is called when the socket is closed (as a result of a TcpSocket::close() call)
 *  @param  socket      Pointer to the socket
 */
void MyConnection::onClosed(Network::TcpSocket *socket)
{
    // show
    std::cout << "myconnection closed" << std::endl;

    // close the channel and connection
    _channel = nullptr;
    _connection = nullptr;

    // stop the loop
    Event::MainLoop::instance()->stop();
}

/**
 *  Method that is called when the peer closed the connection
 *  @param  socket      Pointer to the socket
 */
void MyConnection::onLost(Network::TcpSocket *socket)
{
    // report error
    std::cout << "connection lost" << std::endl;

    // close the channel and connection
    _channel = nullptr;
    _connection = nullptr;

    // stop the loop
    Event::MainLoop::instance()->stop();
}

/**
 *  Method that is called when data is received on the socket
 *  @param  socket      Pointer to the socket
 *  @param  buffer      Pointer to the fill input buffer
 */
void MyConnection::onData(Network::TcpSocket *socket, Network::Buffer *buffer)
{
    // leap out if there is no connection
    if (!_connection) return;

    // let the data be handled by the connection
    size_t bytes = _connection->parse(buffer->data(), buffer->size());

    // shrink the buffer
    buffer->shrink(bytes);
}

/**
 *  Method that is called when data needs to be sent over the network
 *
 *  Note that the AMQP library does no buffering by itself. This means
 *  that this method should always send out all data or do the buffering
 *  itself.
 *
 *  @param  connection      The connection that created this output
 *  @param  buffer          Data to send
 *  @param  size            Size of the buffer
 */
void MyConnection::onData(AMQP::Connection *connection, const char *buffer, size_t size)
{
    // send to the socket
    _socket.write(buffer, size);
}

/**
 *  Method that is called when the connection to AMQP was closed
 *  @param  connection  pointer to connection object
 */
void MyConnection::onClosed(AMQP::Connection *connection)
{
    // report that AMQP connection is closed
    std::cout << "AMQP connection closed" << std::endl;

    // close the underlying socket
    _socket.close();
}

/**
 *  When the connection ends up in an error state this method is called.
 *  This happens when data comes in that does not match the AMQP protocol
 *
 *  After this method is called, the connection no longer is in a valid
 *  state and can be used. In normal circumstances this method is not called.
 *
 *  @param  connection      The connection that entered the error state
 *  @param  message         Error message
 */
void MyConnection::onError(AMQP::Connection *connection, const std::string &message)
{
    // report error
    std::cout << "AMQP Connection error: " << message << std::endl;
}

/**
 *  Method that is called when the login attempt succeeded. After this method
 *  was called, the connection is ready to use
 *
 *  @param  connection      The connection that can now be used
 */
void MyConnection::onConnected(AMQP::Connection *connection)
{
    // show
    std::cout << "AMQP login success" << std::endl;

    // create channel if it does not yet exist
    if (!_channel) _channel = std::unique_ptr<AMQP::Channel>(new AMQP::Channel(connection));
}
