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
    _socket(Event::MainLoop::instance(), this)
{
    // start connecting
    if (_socket.connect(Network::Ipv4Address(ip), 5672)) return;
    
    // failure
    onFailure(&_socket);
}

/**
 *  Destructor
 */
MyConnection::~MyConnection()
{
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
    _connection = std::make_shared<AMQP::Connection>(this, AMQP::Login("guest", "guest"), "/");
    _channel = std::make_shared<AMQP::Channel>(_connection.get());
    
    // install a handler when channel is in error
    _channel->onError([](const char *message) {
        
        std::cout << "channel error " << message << std::endl;
    });
    
    // install a handler when channel is ready
    _channel->onReady([]() {
        
        std::cout << "channel ready" << std::endl;
    });
    
    // we declare a queue, an exchange and we publish a message
    _channel->declareQueue("my_queue").onSuccess([this]() { 
        std::cout << "queue declared" << std::endl; 
        
        // start consuming
        _channel->consume("my_queue").onReceived([](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {
            std::cout << "received: " << message.message() << std::endl;
        });
    });

    // declare an exchange
    _channel->declareExchange().onSuccess([]() { 
        std::cout << "exchange declared" << std::endl; 
    });
    
    // bind queue and exchange
    _channel->bindQueue("my_exchange", "my_queue", "key").onSuccess([this]() {
        std::cout << "queue bound to exchange" << std::endl;
        
        _channel->publish("my_exchange", "key", "just a message");
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

    // set to null
    _channel = nullptr;
    _connection = nullptr;
}

/**
 *  Method that is called when the peer closed the connection
 *  @param  socket      Pointer to the socket
 */
void MyConnection::onLost(Network::TcpSocket *socket)
{
    // report error
    std::cout << "connection lost" << std::endl;
    
    // set to null
    _channel = nullptr;
    _connection = nullptr;
}

/**
 *  Method that is called when data is received on the socket
 *  @param  socket      Pointer to the socket
 *  @param  buffer      Pointer to the fill input buffer
 */
void MyConnection::onData(Network::TcpSocket *socket, Network::Buffer *buffer)
{
    // send what came in
    std::cout << "received: " << buffer->size() << " bytes" << std::endl;
    
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
//    // report what is going on
//    std::cout << "send: " << size << std::endl;
//    
//    for (unsigned i=0; i<size; i++) std::cout << (int)buffer[i] << " ";
//    std::cout << std::endl;
    
    
    // send to the socket
    _socket.write(buffer, size);
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
void MyConnection::onError(AMQP::Connection *connection, const char *message)
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
    if (!_channel) _channel = std::make_shared<AMQP::Channel>(connection);
}

