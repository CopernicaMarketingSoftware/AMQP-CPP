AMQP
====

AMQP is a C++ library for communicating with a RabbitMQ message broker. The 
library can be used to parse incoming data from a RabbitMQ server, and to 
generate frames that can be sent to a RabbitMQ server.

Unlike all other AMQP libraries, this AMQP library does not make a connection to
RabbitMQ by itself, nor does it create sockets and/or performs IO operations. As 
a user of this library, you therefore first need to set up a socket connection 
to RabbitMQ by yourself, and implement a certain interface that you pass to the 
AMQP library and that the library will use for IO operations.

This architecture makes the library extremely flexible: it does not rely on
operating system specific IO calls, and it can be easily integrated into any
event loop. It is fully asynchronous and does not do any blocking (system) calls, 
so it can be used in high performance applications without the need for threads.


ABOUT
=====

This library is created and maintained by Copernica (www.copernica.com), and is 
used inside the MailerQ (www.mailerq.com) application, MailerQ is a tool for 
sending large volumes of email, using AMQP message queues.


HOW TO USE
==========

As we mentioned above, the library does not do any IO by itself, and you need
to pass an object to the library that the library can use for that. So, before
you can even start using the library, you first you need to create a class that
extends from the ConnectionHandler base class. This is a class
with a number of methods that are called by the library every time it wants
to send out data, or when it needs to inform you that an error occured.

````c++
#include <libamqp.h>

class MyConnectionHandler : public AMQP::ConnectionHandler
{
    /**
     *  Method that is called by the AMQP library every time it has data
     *  available that should be sent to RabbitMQ. 
     *  @param  connection  pointer to the main connection object  
     *  @param  data        memory buffer with the data that should be sent to RabbitMQ
     *  @param  size        size of the buffer
     */
    virtual void onData(AMQP::Connection *connection, const char *data, size_t size)
    {
        // @todo 
        //  Add your own implementation, for example by doing a call to the
        //  send() system call. But be aware that the send() call may not
        //  send all data at once, so you also need to take care of buffering
        //  the bytes that could not immediately be sent, and try to send 
        //  them again when the socket becomes writable again
    }
    
    /**
     *  Method that is called by the AMQP library when the login attempt 
     *  succeeded. After this method has been called, the connection is ready 
     *  to use.
     *  @param  connection      The connection that can now be used
     */
    virtual void onConnected(Connection *connection)
    {
        // @todo
        //  add your own implementation, for example by creating a channel 
        //  instance, and start publishing or consuming
    }
    
    /**
     *  Method that is called by the AMQP library when a fatal error occurs
     *  on the connection, for example because data received from RabbitMQ
     *  could not be recognized.
     *  @param  connection      The connection on which the error occured
     *  @param  message         A human readable error message
     */
    virtual void onError(Connection *connection, const std::string &message)
    {
        // @todo
        //  add your own implementation, for example by reporting the error
        //  to the user of your program, log the error, and destruct the 
        //  connection object because it is no longer in a usable state
    }
};
````
After you've implemented the ConnectionHandler class, you can start using
the library by creating a Connection object, and one or more Channel objects:

````c++
// create an instance of your own connection handler
MyConnectionHandler myHandler;

// create a AMQP connection object
AMQP::Connection connection(&myHandler, Login("guest","guest"), "/");

// and create a channel
AMQP::Channel channel(&connection);

// use the channel object to call the AMQP method you like
channel.declareExchange("my-exchange", AMQP::fanout);
channel.declareQueue("my-queue");
channel.bindQueue("my-exchange", "my-queue");
````

A number of remarks about the example above. First you may have noticed that we've
created all objects on the stack. You are of course also free to create them
on the heap with the C++ operator 'new'. That works just as good, and is in real
life code probably more useful as you normally want to keep your handlers, connection 
and channel objects around for a much longer time.

But more importantly, you can see in the example above that we have created the 
channel object directly after we made the connection object, and we also 
started declaring exchanges and queues right away. However, under the hood, a handshake
protocol is executed between the server and the client when the Connection
object is first created. During this handshake procedure it is not permitted to send
other instructions (like opening a channel or declaring a queue). It would therefore have been better 
if we had first waited for the connection to be ready (implement the MyConnectionHandler::onConnected() method), 
and create the channel object only then. But this is not strictly necessary. 
The methods that are called before the handshake is completed are cached by the 
AMQP library and will be executed the moment the handshake is completed and the
connection becomes ready for use.


PARSING INCOMING DATA
=====================

The ConnectionHandler class has a method onData() that is called by the library
every time that it wants to send out data. We've explained that it is up to you to 
implement that method. But what about data in the other direction? How does the 
library receive data back from RabbitMQ?

As we've explained above, the AMQP library does not do any IO by itself 
and it is therefore of course also not possible for the library to receive data from 
a socket. It is again up to you to do this. If, for example, you notice in your 
event loop that the socket that is connected with the RabbitMQ server becomes 
readable, you should read out that data (for example by using the recv() system 
call), and pass the received bytes to the AMQP library. This is done by
calling the parse() method in the Connection object.

The Connection::parse() method gets two parameters, a pointer to a buffer of
data received from RabbitMQ, and a parameter that holds the size of this buffer.
The code snippet below comes from the Connection.h C++ header file.

````c++
/**
 *  Parse data that was recevied from RabbitMQ
 *  
 *  Every time that data comes in from RabbitMQ, you should call this method to parse
 *  the incoming data, and let it handle by the AMQP library. This method returns the number
 *  of bytes that were processed.
 *
 *  If not all bytes could be processed because it only contained a partial frame, you should
 *  call this same method later on when more data is available. The AMQP library does not do
 *  any buffering, so it is up to the caller to ensure that the old data is also passed in that
 *  later call.
 *
 *  @param  buffer      buffer to decode
 *  @param  size        size of the buffer to decode
 *  @return             number of bytes that were processed
 */
size_t parse(char *buffer, size_t size)
{
    return _implementation.parse(buffer, size);
}
````

CHANNELS
========

In the example above you saw that we created two objects: a Connection object, and
on top of that object a Channel object. A channel is a sort of virtual connection 
over a single TCP connection, and you can create many channels that all use the
same TCP connection.

AMQP instructions are always sent over a channel, so before you can send the first 
command to the RabbitMQ server, you first need a channel object. The channel 
object has many methods to send instructions to the RabbitMQ server. It for
example has methods to declare queues and exchanges, to bind and unbind them, 
and to publish and consume messages. You can best take a look at the channel.h 
C++ header file for a list of all available methods. Every method in it is well 
documented.

The constructor of the Channel object gets two parameters: the connection object,
and a pointer to a ChannelHandler object. In the example that we gave above we have 
not yet used this ChannelHandler object. However, in normal circumstances when you 
construct a Channel object, you should also pass a pointer to a ChannelHandler object. 

Just like the ConnectionHandler class, the ChannelHandler class is a base class that 
you should extend and override the virtual methods that you need. The AMQP library 
will call these methods to inform you that an operation has succeeded or has failed.

For example, if you call the channel.declareQueue() method, the AMQP library will
send a message to the RabbitMQ message broker to ask it to declare the
queue. However, because all operations in the library are asynchronous, the
declareQueue() method immediately returns 'true', but it is
then not yet known if the queue was correctly declared. Only after a while,
when the confirmation from the server was received, your ChannelHandler::onQueueDeclared()
method will be called to inform you that the operation was succesful.

Something makes channels a little inconvenient. When an error occurs, the error 
is reported back to the client and ends up in your ChannelHandler::onError() 
method (which is nice), and on top of that the entire channel is closed
(which is not so nice). This means that if you call multiple methods in a row,
and the first method fails, all subsequent methods will not be executed either:

````c++
Channel myChannel(connection, &myHandler);
myChannel.declareQueue("queue-1");
myChannel.declareQueue("queue-2");
myChannel.declareQueue("queue-3");
````

If the first declareQueue() call fails in the example above, your ChannelHandler::onError() 
method will be called once, and the second and third queues would not be created. This can 
be solved by using multiple channels:

````c++
Channel channel1(connection, &myHandler);
Channel channel2(connection, &myHandler);
Channel channel3(connection, &myHandler);
channel1.declareQueue("queue-1");
channel2.declareQueue("queue-2");
channel3.declareQueue("queue-3");
````

Now, if an error occurs with declaring the first queue, it will not have
consequences for the other two calls. But this workaround comes at a small price:
setting up the extra channels require extra instructions to be sent to the
RabbitMQ server, so the network becomes busier (although not much).

Another solution would be to write a handler that only creates the second and
third queue after the earlier queue was succesfully created:

````c++
class MyHandler : public AMQP::ChannelHandler
{
public:
    virtual void onQueueDeclared(Channel *channel, const std::string &name, uint32_t messageCount, uint32_t consumerCount) 
    {
        if (name == "queue-1") channel->declareQueue("queue-2");
        if (name == "queue-2") channel->declareQueue("queue-3");
    }
};

...

MyHandler myHandler;
Channel myChannel(connection, &myHandler);
myChannel.declareQueue("queue-1");

````

But this also has its price: your program now has to wait for the first queue
to be created, before the second instruction can be sent. This is even slower
than the first workaround (in which we set up a different channel for each and 
every instruction).

Let's get back to the ChannelHandler class. It has many methods that you can all
implement - but you do not have to that. All methods in it have a default empty implementation,
so you can only override the ones that you are interested in. When you're writing a
consumer application for example, you probably are only interested in errors that
occur, and in incoming messages:

````c++
#include <libamqp.h>

class MyChannelHandler : public AMQP::ChannelHandler
{
public:
    /**
     *  Method that is called when an error occurs on the channel, and
     *  the channel ends up in an error state
     *  @param  channel     the channel on which the error occured
     *  @param  message     human readable error message
     */
    virtual void onError(AMQP::Channel *channel, const std::string &message)
    {
        // @todo
        //  do something with the error message (like reporting it to the end-user)
        //  and destruct the channel object because it now no longer is usable
    }

    /**
     *  Method that is called when a message has been received on a channel
     *  This message will be called for every message that is received after
     *  you started consuming. Make sure you acknowledge the messages when its
     *  safe to remove them from RabbitMQ (unless you set no-ack option when you
     *  started the consumer)
     *  @param  channel         the channel on which the consumer was started
     *  @param  message         the consumed message
     *  @param  deliveryTag     the delivery tag, you need this to acknowledge the message
     *  @param  consumerTag     the consumer identifier that was used to retrieve this message
     *  @param  redelivered     is this a redelivered message?
     */
    virtual void onReceived(Channel *channel, const Message &message, uint64_t deliveryTag, const std::string &consumerTag, bool redelivered) 
    {
        // @todo
        //  do something with the incoming message
    }
};
````

FLAGS AND TABLES
================

Let's take a closer look at one of the methods in the Channel object to explain
two other concepts of this AMQP library: flags and tables. The method that we
will be looking at is the Channel::declareQueue() method:

````c++
/**
 *  Declare a queue
 * 
 *  If you do not supply a name, a name will be assigned by the server.
 * 
 *  The flags can be a combination of the following values:
 * 
 *      -   durable     queue survives a broker restart
 *      -   autodelete  queue is automatically removed when all connected consumers are gone
 *      -   passive     only check if the queue exist
 *      -   exclusive   the queue only exists for this connection, and is automatically removed when connection is gone
 *    
 *  @param  name        name of the queue
 *  @param  flags       combination of flags
 *  @param  arguments   optional arguments
 */
bool declareQueue(const std::string &name, int flags, const Table &arguments);
````

Many methods in the Channel class support have a parameter named 'flags'. This
is a variable in which you can set a number of options. If you for example
want to create a durable, auto-deleted queue, you should pass in the value
AMQP::durable + AMQP::autodelete.

The declareQueue() method also accepts an arguments parameter, which is of type
Table. This Table object can be used as an associative array to send additional
options to RabbitMQ, that are often custom RabbitMQ extensions to the AMQP 
standard. The Table class is so powerful that it is even possible to build 
complicated, nested structures full of strings, arrays and even other nested 
tables. In reality, you probably only need strings and integers:

````c++
// custom options that are passed to the declareQueue call
Table arguments;
arguments["x-dead-letter-exchange"] = "some-exchange";
arguments["x-message-ttl"] = 3600 * 1000;
arguments["x-expires"] = 7200 * 1000;

// declare the queue
channel.declareQueue("my-queue-name", AMQP::durable + AMQP::autodelete, arguments);
````

PUBLISHING MESSAGES
===================


CONSUMING MESSAGES
==================


WORK IN PROGRESS
================

Almost all AMQP features have been implemented. But the following things might
need additional attention:

    -   ability to set up secure connections (or is this fully done on the IO level)
    -   login with other protocols than login/password
    -   publish confirms

We also need to add more safety checks so that strange or invalid data from 
RabbitMQ does not break the library (although in reality RabbitMQ only sends 
valid data). Also, when we now receive an answer from RabbitMQ that does not
match the request that we earlier sent, we do not report an error (this is also 
an issue that only occurs in theory).

It would be nice to have sample implementations for the ConnectionHandler
class that can be directly plugged into libev, libevent and libuv event loops.

For performance reasons, we need to investigate if we can limit the number of times
an incoming or outgoing messages is copied.

