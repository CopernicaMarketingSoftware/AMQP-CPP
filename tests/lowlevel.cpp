#include <copernica/amqp.h>
#include <copernica/network.h>
#include <iomanip>
#include "amqpbasicframe.h"

// namespaces to use
using namespace std;
using namespace Copernica;

/**
 *  Our own socket handler
 */
class SocketHandler : public Network::TcpHandler
{
private:
    /**
     *  Status of the connection, and substatuses
     *  @var int
     */
    int _status;
    int _connectionStatus;
    int _channelStatus;
    int _exchangeStatus;
    int _qosStatus;
    int _queueStatus;
    int _basicStatus;
    int _deleteStatus;
    int _restStatus;

    AMQP::AMQPBasic _basic;

    /**
     *  Channel we're working on
     *  @var int
     */
    uint16_t _channel;

    std::string _exchangeName;
    std::string _queueName;

public:



    /**
     *  Constructor
     */
    SocketHandler() : _status(0), _connectionStatus(0), _channelStatus(0), _qosStatus(0), _exchangeStatus(0), _queueStatus(0), _basicStatus(0), _deleteStatus(0), _restStatus(0), _basic(), _channel(0), _exchangeName("testexchange"), _queueName("testqueue") {}
    
    /**
     *  Virtual destructor
     */
    virtual ~SocketHandler() {}

    /**
     *  Method that is called when the connection failed
     *  @param  socket      Pointer to the socket
     */
    virtual void onFailure(Network::TcpSocket *socket) 
    {
        cout << "connection failure" << endl;
    }

    /**
     *  Method that is called when the connection timed out (which also is a failure
     *  @param  socket      Pointer to the socket
     */
    virtual void onTimeout(Network::TcpSocket *socket) 
    {
        cout << "connection timeout" << endl;
    }
    
    /**
     *  Method that is called when the connection succeeded
     *  @param  socket      Pointer to the socket
     */
    virtual void onConnected(Network::TcpSocket *socket) 
    {
        if(_status == 0){ // send protocol header frame
            AMQP::ProtocolHeaderFrame frame;
            socket->write(frame.buffer(), frame.size());
        }        
    }
    
    /**
     *  Method that is called when the socket is closed (as a result of a TcpSocket::close() call)
     *  @param  socket      Pointer to the socket
     */
    virtual void onClosed(Network::TcpSocket *socket) 
    {
        cout << "connection closed" << endl;
    }

    /**
     *  Method that is called when the peer closed the connection
     *  @param  socket      Pointer to the socket
     */
    virtual void onLost(Network::TcpSocket *socket) 
    {
        cout << "connection lost" << endl;
    }
    
    /**
     *  Method that is called when data is received on the socket
     *  @param  socket      Pointer to the socket
     *  @param  buffer      Pointer to the filled input buffer
     */
    virtual void onData(Network::TcpSocket *socket, Network::Buffer *buffer) 
    {
        // try getting a frame
        auto frame = AMQP::Frame::decode(buffer->data(), buffer->length());
        if(frame == nullptr)
        {
            std::cout << "error decoding received frame" << std::endl;

            return;
        }
        if(_status != 5) buffer->shrink(frame->totalSize());
        if(_status == 0)
        {
            // the connection hasn't been set up, do this now.
            setupConnection(*socket, *buffer, frame);
        }
        if(_status == 1)
        {
            // time to setup the channel
            setupChannel(*socket, *buffer, frame);
        }
        if(_status == 2)
        {
            // time to do the QOS stuff
            setupQOS(*socket, *buffer, frame);
        }
        if(_status == 3)
        {
            // setup the exchange
            setupExchange(*socket, *buffer, frame);
        }
        if(_status == 4)
        {
            // setup the queue
            setupQueue(*socket, *buffer, frame);
        }
        if(_status == 5)
        {
            // publish testing etc
            testBasicFunctionalities(*socket, *buffer, frame);
        }
        if(_status == 6)
        {
            testRestStuff(*socket, *buffer, frame);
        }
        if(_status == 7)
        {
            // start deleting everything.
            deleteAMQP(*socket, *buffer, frame);
        }
        if(_status == 8)
        {
            socket->close();
        }
    }


    void testRestStuff(Network::TcpSocket &socket, Network::Buffer &buffer, std::shared_ptr<AMQP::Frame> &frame)
    {
        if(_restStatus == 0)
        {
            std::cout << "rest testing";
            std::string consumertag = "";
            auto *sendframe = new AMQP::BasicCancelFrame(_channel, consumertag, false);
            socket.write(sendframe->buffer(), sendframe->totalSize());
            _restStatus++;
            delete sendframe;
            return; // wait for response
        }
        if(_restStatus == 1)
        {
            auto f = std::dynamic_pointer_cast<AMQP::BasicCancelOKFrame>(frame);
            if(f == nullptr){ std::cout<<"Error casting to BasicCancelOKFrame, should work"<<std::endl; return; }
            _restStatus = 5;
        }
        if(_restStatus == 2)
        {  
            auto *sendframe = new AMQP::BasicRecoverFrame(_channel, true);
            socket.write(sendframe->buffer(), sendframe->totalSize());
            _restStatus++;
            delete sendframe;
            return; // wait for response
        }
        if(_restStatus == 3)
        {
            auto f = std::dynamic_pointer_cast<AMQP::BasicRecoverOKFrame>(frame);
            if(f == nullptr){ std::cout<<"Error casting to BasicRecoverOKFrame, should work"<<std::endl; return; }
            _restStatus++;
        }
        if(_restStatus == 4)
        {  
            auto *sendframe = new AMQP::BasicRecoverAsyncFrame(_channel, true);
            socket.write(sendframe->buffer(), sendframe->totalSize());
            _restStatus++;
            delete sendframe;
            // recover async does not wait for response. 
        }
        if(_restStatus == 5)
        {
            declareQueue(socket, buffer);
            _restStatus++;
            return; //wait
        }
        if(_restStatus == 6)
        {
            waitDeclareQueueOK(buffer, frame);
            _restStatus = 20;
        }

        if(_restStatus == 20)
        {
            purgeQueue(socket, buffer);
            _restStatus++;
            return;
        }

        if(_restStatus == 21)
        {
            waitPurgeQueueOK(buffer, frame);
            _restStatus = 7;
        }


        if(_restStatus == 7)
        {
            auto *sendframe = new AMQP::TransactionSelectFrame(_channel);
            socket.write(sendframe->buffer(), sendframe->totalSize());
            _restStatus++;
            delete sendframe;
            return; // wait for response
        }
        if(_restStatus == 8)
        {
            auto f = std::dynamic_pointer_cast<AMQP::TransactionSelectOKFrame>(frame);
            if(f == nullptr){ std::cout<<"Error casting to TransactionSelectOKFrame, should work"<<std::endl; return; }
            _restStatus++;
        }
        if(_restStatus == 9)
        {  
            auto *sendframe = new AMQP::TransactionCommitFrame(_channel);
            socket.write(sendframe->buffer(), sendframe->totalSize());
            _restStatus++;
            delete sendframe; 
            return; // wait for response
        }
        if(_restStatus == 10)
        {
            auto f = std::dynamic_pointer_cast<AMQP::TransactionCommitOKFrame>(frame);
            if(f == nullptr){ std::cout<<"Error casting to TransactionCommitOKFrame, should work"<<std::endl; return; }
            _restStatus++;
        }
        if(_restStatus == 11)
        {
            auto *sendframe = new AMQP::TransactionRollbackFrame(_channel);
            socket.write(sendframe->buffer(), sendframe->totalSize());
            _restStatus++;
            delete sendframe;
            return; // wait for response
        }
        if(_restStatus == 12)
        {
            auto f = std::dynamic_pointer_cast<AMQP::TransactionRollbackOKFrame>(frame);
            if(f == nullptr){ std::cout<<"Error casting to TransactionRollbackOKFrame, should work"<<std::endl; return; }
            _restStatus++;
        }
        


        std::cout << " ... DONE" << std::endl;
        _status++;
    }


    int deliverHeaderBody(Network::TcpSocket& socket, Network::Buffer& buffer, std::shared_ptr<AMQP::Frame> &frame)
    {
        int lastDeliveryTag = 0;
        while(buffer.length() > 0)
        {
            if(_basicStatus != 20)
            {
                frame = AMQP::Frame::decode(buffer.data(), buffer.length());
                buffer.shrink(frame->totalSize());
            } 
            auto f = std::dynamic_pointer_cast<AMQP::BasicDeliverFrame>(frame);
            if(f == nullptr){ std::cout<<"Error casting to BasicDeliverFrame, should work"<<std::endl; return 0; }


            lastDeliveryTag = f->deliveryTag();

            frame = AMQP::Frame::decode(buffer.data(), buffer.length());
            auto f2 = std::dynamic_pointer_cast<AMQP::BasicHeaderFrame>(frame);
            if(f2 == nullptr){ std::cout<<"Error casting to BasicHeaderFrame, should work"<<std::endl; return 0; }
            buffer.shrink(f2->totalSize());

            int bytesRead = f2->bodySize();

            while(bytesRead > 0)
            {
                frame = AMQP::Frame::decode(buffer.data(), buffer.length());
                auto f3 = std::dynamic_pointer_cast<AMQP::BodyFrame>(frame);
                if(f3 == nullptr){ std::cout<<"Error casting to BodyFrame, should work"<<std::endl; return 0; }
                buffer.shrink(f3->totalSize());

                auto payload = f3->payload();
                bytesRead -= payload.size();
            }
        }
        return lastDeliveryTag;
    }



    void testBasicFunctionalities(Network::TcpSocket& socket, Network::Buffer& buffer, std::shared_ptr<AMQP::Frame> &frame)
    {
        int lastDeliveryTag = 0;
        if(_basicStatus == 0)
        {   // basic publish
            std::cout << "basic testing";
            _basic.sendPublish(socket);
            _basicStatus++;
        }
        
        if(_basicStatus == 1)
        {   // content header
            _basic.sendHeader(socket);
            _basicStatus++;
            
        }
        if(_basicStatus == 2)
        {   // content body
            for(int i = 0 ; i < 10; i++) { _basic.sendBody(socket); }
            _basicStatus++;
            
        }
        if(_basicStatus == 3)
        {   // basic consume frame
            _basic.sendConsume(socket);
            _basicStatus++;
            return;
        }
        

        if(_basicStatus == 4)
        {
            _basic.waitConsumeOK(buffer);
            _basicStatus++;
        }

        if(_basicStatus == 5)
        {   // received consumeok frame, let's work on those basic deliver, content header, content body frames.
            _basic.waitDeliver(buffer);
            _basic.waitHeader(buffer);
            _basic.waitBody(buffer);
            _basicStatus++;
        }

        // done reading all incoming messages, ack them.
        if(_basicStatus == 6)
        {
            _basic.sendAck(socket);
            _basicStatus+=2;
        }

        if(_basicStatus == 8)
        {   // send a basic get frame
            _basic.sendGet(socket);
            _basicStatus++;
            return;     // wait for response
        }

        
        if(_basicStatus == 9)
        {   // expect a basic-get-empty frame (at 5 & 6 we've cleared the queue)

            _basic.waitGetEmpty(buffer);
            _basicStatus ++;
        }


        if(_basicStatus == 10)
        {   // send some more data to the queue, to test basicgetok
            for(int i = 0; i < 5; i++){
                _basic.sendPublish(socket);
                _basic.sendHeader(socket);
                for(int i = 0; i < 10; i++) {  _basic.sendBody(socket);  }
            }
            _basicStatus++;
        }

        
        if(_basicStatus == 11)
        {   // send a basic get frame
            _basic.sendGet(socket);
            _basicStatus++;
            return;     // wait for response
        }

        
        
        if(_basicStatus == 12)
        {   // expect a basic-get-empty frame (at 5 & 6 we've cleared the queue)
            _basic.waitGetOK(buffer);
            _basic.waitHeader(buffer);
            _basic.waitBody(buffer);
            _basicStatus++;
        }
        
        if(_basicStatus == 13)
        {   // received consumeok frame, let's work on those basic deliver, content header, content body frames.
            while(buffer.length() > 0)
            {
                _basic.waitDeliver(buffer);
                _basic.waitHeader(buffer);
                _basic.waitBody(buffer);
            }
            _basicStatus++;
        }

        if(_basicStatus == 14)
        {
            // add some more messages to the queue
            _basic.sendPublish(socket);
            _basic.sendHeader(socket, 9);
            _basic.sendBody(socket);
            _basicStatus++;
        }

        if(_basicStatus == 15)
        {
            // channel flow frame
            auto *sendframe2 = new AMQP::ChannelFlowFrame(_channel, true);
            socket.write(sendframe2->buffer(), sendframe2->totalSize());
            _basicStatus++;
            delete sendframe2;
            return; // wait for response
        }

        
        if(_basicStatus == 16)
        {
            // check if we've received a channelflowokframe
            auto f = std::dynamic_pointer_cast<AMQP::ChannelFlowOKFrame>(frame);
            if(f == nullptr){ std::cout<<"Error casting to channelflowokframe, should work"<<std::endl; return; }
            buffer.shrink(frame->totalSize());
            _basicStatus++;
        }

        if(_basicStatus == 17)
        {
            _status++;
            std::cout << " ... DONE" << std::endl;
            return;
        }
        
    }

    void purgeQueue(Network::TcpSocket& socket, Network::Buffer& buffer)
    {
        auto *sendframe = new AMQP::QueuePurgeFrame(_channel, _queueName);
        socket.write(sendframe->buffer(), sendframe->totalSize());
        delete sendframe;
    }

    void waitPurgeQueueOK( Network::Buffer& buffer, std::shared_ptr<AMQP::Frame> &frame)
    {
        auto f = std::dynamic_pointer_cast<AMQP::QueuePurgeOKFrame>(frame);
        if(f == nullptr){ std::cout<<"Error casting to QueuePurgeOKFrame, should work"<<std::endl; return; }
        buffer.shrink(frame->totalSize());
    }

    void deleteAMQP(Network::TcpSocket& socket, Network::Buffer& buffer, std::shared_ptr<AMQP::Frame> &frame)
    {
        // queue deletion stuff

        // for purge testing, let's add a message to the queue (in case it is empty)
        if(_deleteStatus == 0)
        {   // nothing deleted so far
            // queue purge
            auto *sendframe = new AMQP::QueuePurgeFrame(_channel, _queueName);
            socket.write(sendframe->buffer(), sendframe->totalSize());
            _deleteStatus++;
            delete sendframe;
            return;     // wait for response
        }
        if(_deleteStatus == 1)
        {   // nothing deleted so far
            // queue purge ok
            auto f = std::dynamic_pointer_cast<AMQP::QueuePurgeOKFrame>(frame);
            if(f == nullptr){ std::cout<<"Error casting to QueuePurgeOKFrame, should work"<<std::endl; return; }
            buffer.shrink(frame->totalSize());

            _deleteStatus++;
        }
        
        if(_deleteStatus == 2)
        {   // nothing deleted so far
            std::cout << "cleanup";
            // unbind queue
            auto *sendframe = new AMQP::QueueUnbindFrame(_channel, _queueName, _exchangeName);
            socket.write(sendframe->buffer(), sendframe->totalSize());
            _deleteStatus++;
            delete sendframe;
            return;     // wait for response
        }
        if(_deleteStatus == 3)
        {   // nothing deleted so far
            // unbind queue ok
            auto f = std::dynamic_pointer_cast<AMQP::QueueUnbindOKFrame>(frame);
            if(f == nullptr){ std::cout<<"Error casting to QueueUnbindOKFrame, should work"<<std::endl; return; }
            buffer.shrink(frame->totalSize());

            _deleteStatus++;
        }
        if(_deleteStatus == 4)
        {   // nothing deleted so far
            // queue delete
            auto *sendframe = new AMQP::QueueDeleteFrame(_channel, _queueName);
            socket.write(sendframe->buffer(), sendframe->totalSize());
            _deleteStatus++;
            delete sendframe;
            return;     // wait for response
        }
        if(_deleteStatus == 5)
        {   // nothing deleted so far
            //// queue delete ok
            auto f = std::dynamic_pointer_cast<AMQP::QueueDeleteOKFrame>(frame);
            if(f == nullptr){ std::cout<<"Error casting to QueueDeleteOKFrame, should work"<<std::endl; return; }
            buffer.shrink(frame->totalSize());

            _deleteStatus++;
        }

        //exchange deletion
        if(_deleteStatus == 6)
        {   // send exchange delete
            auto *sendframe = new AMQP::ExchangeDeleteFrame(_channel, _exchangeName);
            socket.write(sendframe->buffer(), sendframe->totalSize());
            _deleteStatus++;
            delete sendframe;
            return;     // wait for response
        }
        if(_deleteStatus == 7)
        {   // expect exchangedeleteok
            auto f = std::dynamic_pointer_cast<AMQP::ExchangeDeleteOKFrame>(frame);
            if(f == nullptr){ std::cout<<"Error casting to ExchangeDeleteOKFrame, should work"<<std::endl; return; }
            buffer.shrink(frame->totalSize());

            _deleteStatus++;
        }

        // channel close
        if(_deleteStatus == 8)
        {   // send channel close
            std::string replyText = "200";
            auto *sendframe = new AMQP::ChannelCloseFrame(_channel, 200, replyText);
            socket.write(sendframe->buffer(), sendframe->totalSize());
            _deleteStatus++;

            delete sendframe;
            return;     // wait for response
        }
        if(_deleteStatus == 9)
        {   // expect channelcloseok
            auto f = std::dynamic_pointer_cast<AMQP::ChannelCloseOKFrame>(frame);
            if(f == nullptr){ std::cout<<"Error casting to ChannelCloseOKFrame, should work"<<std::endl; return; }
            buffer.shrink(frame->totalSize());

            _deleteStatus++;
        }

        // connection close
        if(_deleteStatus == 10)
        {   // send exchange delete
            std::string replyText = "200";
            auto *sendframe = new AMQP::ConnectionCloseFrame(0, 200, replyText);
            socket.write(sendframe->buffer(), sendframe->totalSize());
            _deleteStatus++;

            delete sendframe;
            return;     // wait for response
        }

        if(_deleteStatus == 11)
        {   // expect exchangedeleteok
            auto f = std::dynamic_pointer_cast<AMQP::ConnectionCloseOKFrame>(frame);
            if(f == nullptr){ std::cout<<"Error casting to ConnectionCloseOKFrame, should work"<<std::endl; return; }
            buffer.shrink(frame->totalSize());

            _deleteStatus++;
            _status++;

            std::cout << " ... DONE" << std::endl;
        }
    }


    void declareQueue(Network::TcpSocket& socket, Network::Buffer& buffer)
    {
        auto *sendframe = new AMQP::QueueDeclareFrame(_channel, _queueName, false, true, false, false, false);
        socket.write(sendframe->buffer(), sendframe->totalSize());
        _queueStatus++;
        delete sendframe;
    }

    void waitDeclareQueueOK(Network::Buffer& buffer, std::shared_ptr<AMQP::Frame> &frame)
    {
        auto f = std::dynamic_pointer_cast<AMQP::QueueDeclareOKFrame>(frame);
        if(f == nullptr){ std::cout<<"Error casting to QueueDeclareOKFrame, should work"<<std::endl; return; }
        buffer.shrink(frame->totalSize());
        _queueStatus++;
    }

    void setupQueue(Network::TcpSocket& socket, Network::Buffer& buffer, std::shared_ptr<AMQP::Frame> &frame)
    {
        if(_queueStatus == 0)
        {   // nothing has been done in the queue world
            std::cout << "set up queue " << _queueName;
            // send a queue declare frame
            declareQueue(socket, buffer);
            return; //wait for response
        }
        if(_queueStatus == 1)
        {
            waitDeclareQueueOK(buffer, frame);
        }
        if(_queueStatus == 2)
        {
            auto *sendframe = new AMQP::QueueBindFrame(_channel, _queueName, _exchangeName);
            socket.write(sendframe->buffer(), sendframe->totalSize());
            _queueStatus++;
            delete sendframe;
            return; // wait for response
        }
        if(_queueStatus == 3)
        {
            auto f = std::dynamic_pointer_cast<AMQP::QueueBindOKFrame>(frame);
            if(f == nullptr){ std::cout<<"Error casting to QueueBindOKFrame, should work"<<std::endl; return; }
            buffer.shrink(frame->totalSize());

            _queueStatus++;
            _status++;
            std::cout << " ... DONE" << std::endl;
        }
    }









    void setupQOS(Network::TcpSocket& socket, Network::Buffer& buffer, std::shared_ptr<AMQP::Frame> &frame)
    {
        if(_qosStatus == 0)
        {   // nothing has been done in the qos world
            std::cout << "set up qos";
            // send the basic qos packet
            auto *sendframe = new AMQP::BasicQosFrame(_channel, 0, 3, false);
            socket.write(sendframe->buffer(), sendframe->totalSize());
            _qosStatus++;
            delete sendframe;
            return; // we wait for a response
        }
        if(_qosStatus == 1)
        {
            auto f = std::dynamic_pointer_cast<AMQP::BasicQosOKFrame>(frame);
            if(f == nullptr){ std::cout<<"Error casting to basicqosokframe, should work"<<std::endl; return; }
            buffer.shrink(frame->totalSize());
            _status++;
            std::cout << " ... DONE" << std::endl;
        }
    }












    void setupChannel(Network::TcpSocket& socket, Network::Buffer& buffer, std::shared_ptr<AMQP::Frame> &frame)
    {
        if(_channelStatus == 0)
        {   // nothing has been done in the channel world.
            std::cout << "set up channel";
            // we send the channel open frame
            _channel = 1;
            auto *sendframe = new AMQP::ChannelOpenFrame(_channel);
            socket.write(sendframe->buffer(), sendframe->totalSize());
            _channelStatus++;

            delete sendframe;
            return; // we wait for the response
        }
        if(_channelStatus == 1)
        {
            // check if we've received a channel open ok response
            auto f = std::dynamic_pointer_cast<AMQP::ChannelOpenOKFrame>(frame);
            if(f == nullptr){ std::cout<<"Error casting to channelopenokframe, should work"<<std::endl; return; }
            buffer.shrink(frame->totalSize());

            _channelStatus++;
            // received a response, send a new frame
        }
        if(_channelStatus == 2)
        {
            // channel flow frame
            auto *sendframe2 = new AMQP::ChannelFlowFrame(_channel, true);
            socket.write(sendframe2->buffer(), sendframe2->totalSize());
            _channelStatus++;
            delete sendframe2;
            return; // wait for response
        }
        if(_channelStatus == 3)
        {
            // check if we've received a channelflowokframe
            auto f = std::dynamic_pointer_cast<AMQP::ChannelFlowOKFrame>(frame);
            if(f == nullptr){ std::cout<<"Error casting to channelflowokframe, should work"<<std::endl; return; }
            buffer.shrink(frame->totalSize());
            _status++;
            _channelStatus++;

            std::cout << " ... DONE" << std::endl;
            return;
        }

    }











    void setupExchange(Network::TcpSocket& socket, Network::Buffer& buffer, std::shared_ptr<AMQP::Frame> &frame)
    {
        if(_exchangeStatus == 0)
        {   // nothing has been done in the exchange world.
            std::cout << "set up exchange";
            // create an exchange declare frame
            const AMQP::Table *t = new AMQP::Table();
            std::string type = "direct";
            auto *sendframe = new AMQP::ExchangeDeclareFrame(_channel, _exchangeName, type, false, true, false, *t);

            socket.write(sendframe->buffer(), sendframe->totalSize());
            _exchangeStatus++;
            
            delete t;
            delete sendframe;
            return; // await response
        }
        if(_exchangeStatus == 1)
        {   // expect an exchangedeclareokframe
            auto f = std::dynamic_pointer_cast<AMQP::ExchangeDeclareOKFrame>(frame);
            if(f == nullptr){ std::cout<<"Error casting to exchangedeclareokframe, should work"<<std::endl; return; }
            buffer.shrink(frame->totalSize());
            _status++;
            _exchangeStatus++;
            std::cout << " ... DONE" << std::endl;
            return;
        }
    }












    void setupConnection(Network::TcpSocket& socket, Network::Buffer& buffer, std::shared_ptr<AMQP::Frame> &frame)
    {
        // received connectionStart frame, send connectionStartOK frame
        if(_connectionStatus == 0)
        { 
            std::cout << "set up connection";
            auto f = std::dynamic_pointer_cast<AMQP::ConnectionStartFrame>(frame);
            if(f == nullptr)
            {
                std::cout << "error casting to connectionstartframe, should work??" << std::endl;
                return;
            }
            buffer.shrink(frame->totalSize());
            AMQP::Table *properties = new AMQP::Table();

            std::string mechanism = "PLAIN";
            std::string *response = new std::string("\0micha\0micha", 12);
            std::string locales = "en_US";
            auto *sendframe = new AMQP::ConnectionStartOKFrame(_channel, *properties, mechanism, *response, locales);
            socket.write(sendframe->buffer(), sendframe->totalSize());

            delete response;
            delete sendframe;
            delete properties;
            _connectionStatus++;
            return; // we wait for a response
        }  

        // received connectionTuneFrame, probably
        if(_connectionStatus == 1)
        {
            auto f = std::dynamic_pointer_cast<AMQP::ConnectionTuneFrame>(frame);
            if(f == nullptr)
            {
                std::cout << "error casting to connectiontuneframe, should work??" << std::endl;
                return;
            }
            uint16_t channelMax = 10;
            uint32_t frameMax = 131072;
            uint16_t heartbeat = 0;
            auto *conopframe =  new AMQP::ConnectionTuneOKFrame(_channel, channelMax, frameMax, heartbeat);
            socket.write(conopframe->buffer(), conopframe->totalSize());
            _connectionStatus++;

            delete conopframe;

            // we respond to the server, now send a new frame
        }
        if(_connectionStatus == 2)
        {
            // after sending the tuneok frame, send the connection open frame
            std::string vhost = "/";
            // rest of the fields are deprecated, the constructor handles that for us

            auto *conopframe = new AMQP::ConnectionOpenFrame(_channel, vhost);
            socket.write(conopframe->buffer(), conopframe->totalSize());
            _connectionStatus++;

            delete conopframe;
            return; // we await a response
        }
        if(_connectionStatus == 3)
        {
            // we probably received the connection open ok frame
            buffer.shrink(frame->totalSize());
            _status++;
            std::cout << " ... DONE" << std::endl;
        }
    }
    
    /**
     *  Method that is called when the internal buffers are emptied and the socket
     *  is in a writable state again. It is also called after a call to TcpSocket::wait()
     */
    virtual void onWritable(Network::TcpSocket *socket) 
    {
        
        cout << "socket writable" << endl;
    }
};

/**
 *  Main procedure
 *  @param  argc
 *  @param  argv
 *  @return int
 */
int main(int argc, const char *argv[])
{
    // our own handler
    SocketHandler tcphandler;
    
    // create a socket
    Network::TcpSocket socket(Event::MainLoop::instance(), &tcphandler);
    
    // connect to the socket
    socket.connect(Network::Ipv4Address("127.0.0.1"), 5672);
    
    std::cout << "start main loop" << std::endl;

    // run the main event loop
    Event::MainLoop::instance()->run();

    std::cout << "exited main loop" << std::endl;
    
    // done
    return 0;
}
