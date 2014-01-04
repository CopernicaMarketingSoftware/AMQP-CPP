#include <copernica/network.h>
#include "../amqp.h"
#include <iostream>

using namespace Copernica;
using namespace AMQP;
using namespace Network;

namespace Copernica { namespace AMQP{ 
class TcpSocket : 
    public AMQP::TcpConnection,
    public Network::TcpHandler,
    public AMQP::ChannelHandler,
    public AMQP::ExchangeHandler,
    public Event::TimerHandler
{
private:
    /**
     *  AMQP connection object
     *  @var connection
     */
    Connection *_connection;


public:
    /**
     *  tcpsocket to sent data to, received data from
     *  @var TcpSocket
     */
    Network::TcpSocket *_socket;

    TcpSocket(Event::Loop *loop)
    {
        _socket = new Network::TcpSocket(loop, this);
    }

    /**
     *  Destructor
     */
    virtual ~TcpSocket() {}


    /**
     *  TcpConnection implementation
     */

    /**
     *  Send data to the connection
     *
     *  Note that the AMQP library does not do any output buffering, so the return value
     *  of this method should always be identical to the size of the buffer!
     *
     *  @param  buffer      Data to send
     *  @param  size        Size of the buffer
     *  @return             Number of bytes actually sent, or -1 on failure
     */
    virtual ssize_t send(const char *buffer, size_t size)
    {
        return _socket->write(buffer, size);
    }

    /**
     *  Close the connection
     *  @return bool        Was the connection closed successully?
     */
    virtual bool close()
    {
        _socket->close();
    }


    /**                            **
     *  Connection implementation  **
     *                              */

    virtual void onConnected(Network::TcpSocket *socket)
    {
        std::cout << "connection started, create connection object" << std::endl;
        _connection = new AMQP::Connection(this);
        

        Table *t = new Table();
        std::string mechanism = "PLAIN";
        std::string locale="en_US";
        std::string *credentials = new std::string("\0micha\0micha", 12);
        std::string vhost = "/";
        _connection->setProperties(*t);
        _connection->setMechanism(mechanism);
        _connection->setLocale(locale);
        _connection->setCredentials(*credentials);

        _connection->setChannelMax(10);
        _connection->setFrameMax(10000);
        // heartbeat every 5 seconds
        _connection->setHeartbeat(5);

        _connection->setVhost(vhost);
    }

    virtual void onData(Network::TcpSocket *socket, Network::Buffer *buffer)
    {
        std::cout << "data received" << std::endl;
        size_t bytesRead = _connection->parse(buffer->data(), buffer->length());
        buffer->shrink(bytesRead);
    }


    /**                                         **
     *  ChannelHandler method implementations   **
     *                                          */

    /**
     *  Method that is called when the channel was succesfully created.
     *  Only after the channel was created, you can use it for subsequent messages over it
     *  @param  channel
     */
    virtual void onReady(Channel *channel)
    {
        Exchange exchange(channel, this);
        //exchange.setName("bla");
        //exchange.declare();

    }

    /**
     *  Method that is called when an error occured on a channel.
     *  @param  channel
     */
    virtual void onError(Channel *channel)
    {}

    /**
     *  Method that is called when the QOS was succesfully set
     *  @param channel
     */
    virtual void onQos(Channel *channel)
    {}

    /**
     *  Method that is called when a rollback has succeeded
     *  @param channel
     */
    virtual void onRollback(Channel *channel)
    {}


    /**                                         **
     *  ExchangeHandler method implementations  **
     *                                          */

    /**
     *  The exchange was correctly declared
     *  @param  exchange
     */
    virtual void onDeclared(Exchange *exchange)
    {
        std::cout << "exchange has been declared" << std::endl;
    }

    /**
     *  The exchange could not be declared
     *  @param  exchange
     */
    virtual void onNotDeclared(Exchange *exchange) {}

    /**
     *  The exchange was correctly deleted
     *  @param  exchange
     */
    virtual void onDeleted(Exchange *exchange) {}

    /**
     *  The exchange could not be removed
     *  @param  exchange
     */
    virtual void onNotDeleted(Exchange *exchange) {}

    /**
     *  The exchange was correctly bound
     *  @param  exchange
     */
    virtual void onBound(Exchange *exchange) {}

    /**
     *  The bind call failed
     *  @param  exchange
     */
    virtual void onNotBound(Exchange *exchange) {}

};
}}



int main()
{
    Copernica::AMQP::TcpSocket *tcpHandler = new Copernica::AMQP::TcpSocket(Event::MainLoop::instance());

    //tcpHandler._socket = new Copernica::Network::Socket(Event::MainLoop::instance(), &tcpHandler);
    tcpHandler->_socket->connect(Ipv4Address("127.0.0.1"), 5672);
    std::cout << "run loop." << std::endl;
    Event::MainLoop::instance()->run(); 
    std::cout << "dropped out of loop" << std::endl;

    //MyIoHandler handler(&socket);
    //MyChannelHandler chanHandler;
    
    //Amqp amqp(tcpconnection, handler);
    
    
    return 0;
}


