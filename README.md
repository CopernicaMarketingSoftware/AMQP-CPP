AMQP-CPP
========

AMQP-CPP is a C++ library for communicating with a RabbitMQ message broker. The 
library can be used to parse incoming data from a RabbitMQ server, and to 
generate frames that can be sent to a RabbitMQ server.

Unlike all other AMQP libraries, this AMQP-CPP library does not make a connection to
RabbitMQ by itself, nor does it create sockets and/or performs IO operations. As 
a user of this library, you first need to set up a socket connection 
to RabbitMQ by yourself, and implement a certain interface that you pass to the 
AMQP-CPP library and that the library will use for IO operations.

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
#include <amqpcpp.h>

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
    virtual void onConnected(AMQP::Connection *connection)
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
    virtual void onError(AMQP::Connection *connection, const std::string &message)
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
on the heap with the C++ operator 'new'. That works just as well, and is in real
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

As we've explained above, the AMQP-CPP library does not do any IO by itself 
and it is therefore of course also not possible for the library to receive data from 
a socket. It is again up to you to do this. If, for example, you notice in your 
event loop that the socket that is connected with the RabbitMQ server becomes 
readable, you should read out that data (for example by using the recv() system 
call), and pass the received bytes to the AMQP-CPP library. This is done by
calling the parse() method in the Connection object.

The Connection::parse() method gets two parameters, a pointer to a buffer of
data received from RabbitMQ, and a parameter that holds the size of this buffer.
The code snippet below comes from the Connection.h C++ header file.

````c++
/**
 *  Parse data that was recevied from RabbitMQ
 *  
 *  Every time that data comes in from RabbitMQ, you should call this method to parse
 *  the incoming data, and let it handle by the AMQP-CPP library. This method returns the number
 *  of bytes that were processed.
 *
 *  If not all bytes could be processed because it only contained a partial frame, you should
 *  call this same method later on when more data is available. The AMQP-CPP library does not do
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

In the example you saw that we created a channel object. A channel is a virtual 
connection over a single TCP connection, and it is possible to create many channels 
that all use the same TCP connection.

AMQP instructions are always sent over a channel, so before you can send the first 
command to the RabbitMQ server, you first need a channel object. The channel 
object has many methods to send instructions to the RabbitMQ server. It for
example has methods to declare queues and exchanges, to bind and unbind them, 
and to publish and consume messages. You can best take a look at the channel.h 
C++ header file for a list of all available methods. Every method in it is well 
documented.

The constructor of the Channel object accepts two parameters: the connection object,
and a pointer to a ChannelHandler object. In the example we did 
not use this ChannelHandler object. However, in normal circumstances, you should 
always pass a pointer to a ChannelHandler object every time you construct a channel. 

Just like the ConnectionHandler class, the ChannelHandler class is a base class that 
you can extend to override the virtual methods you need. The AMQP library 
will call these methods to inform you that an operation on the channel has succeeded 
or has failed.

For example, if you call the channel.declareQueue() method, the AMQP-CPP library will
send a message to the RabbitMQ message broker to ask it to declare the
queue. However, because all operations in the library are asynchronous, the
declareQueue() method immediately returns 'true', although it is at that time
not yet known whether the queue was correctly declared. Only after a while,
after the instruction has reached the server, and the confirmation from the server 
has been sent back to the client, your ChannelHandler::onQueueDeclared()
method will be called to inform you that the operation was succesful.

It is important to realize that any error that occurs on a channel, will
invalidate the entire channel,. including all subsequent instructions that
were already sent over it. This means that if you call multiple methods in a row,
and the first method fails, all subsequent methods will not be executed either:

````c++
Channel myChannel(connection, &myHandler);
myChannel.declareQueue("my-queue");
myChannel.declareExchange("my-exchange");
````

If the first declareQueue() call fails in the example above, your ChannelHandler::onError() 
method will be called after a while to report this failure. And although the 
second instruction to declare an exchange has already been sent to the server, it will be 
ignored because the channel was already in an invalid state after the first failure.

You can overcome this by using multiple channels:

````c++
Channel channel1(connection, &myHandler);
Channel channel2(connection, &myHandler);
channel1.declareQueue("my-queue");
channel2.declareQueue("my-exchange");
````

Now, if an error occurs with declaring the queue, it will not have
consequences for the other call. But this comes at a small price:
setting up the extra channel requires and extra instruction to be sent to the
RabbitMQ server, so some extra bytes are sent over the network,
and some additional resources in both the client application and the
RabbitMQ server are used (although this is all very limited).

Let's get back to the ChannelHandler class. It has many methods that you can 
implement - all of which are optional. All methods in it have a default empty implementation,
so you can choose to only override the ones that you are interested in. When you're 
writing a consumer application for example, you probably are only interested in 
errors that occur, and in incoming messages:

````c++
#include <amqpcpp.h>

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
    virtual void onReceived(AMQP::Channel *channel, const AMQP::Message &message, uint64_t deliveryTag, const std::string &consumerTag, bool redelivered) 
    {
        // @todo
        //  do something with the incoming message
    }
};
````

FLAGS AND TABLES
================

Let's take a closer look at one method in the Channel object to explain
two other concepts of this AMQP-CPP library: flags and tables. The method that we
will be looking at is the Channel::declareQueue() method - but we could've
picked a different method too because flags and
tables are used by many methods.

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
bool declareQueue(const std::string &name, int flags, const AMQP::Table &arguments);
bool declareQueue(const std::string &name, const AMQP::Table &arguments);
bool declareQueue(const std::string &name, int flags = 0);
bool declareQueue(int flags, const AMQP::Table &arguments);
bool declareQueue(const AMQP::Table &arguments);
bool declareQueue(int flags = 0);
````

As you can see, the method comes in many forms, and it is up to you to choose
the one that is most appropriate. We now take a look at the most complete 
one, the method with three parameters.

Many methods in the Channel class accept an integer parameter named 'flags'.
This is a variable in which you can set a number of options, by summing up
all the options that are described in the documentation. If you for example
want to create a durable, auto-deleted queue, you can pass in the value
AMQP::durable + AMQP::autodelete.

The declareQueue() method also accepts a parameter named 'arguments', which is of type
Table. This Table object can be used as an associative array to send additional
options to RabbitMQ, that are often custom RabbitMQ extensions to the AMQP 
standard. For a list of all supported arguments, take a look at the documentation
on the RabbitMQ website. With every new RabbitMQ release more features, and
supported arguments are added.

The Table class is a very powerful class that enables you to build 
complicated, deeply nested structures full of strings, arrays and even other 
tables. In reality, you only need strings and integers.

````c++
// custom options that are passed to the declareQueue call
AMQP::Table arguments;
arguments["x-dead-letter-exchange"] = "some-exchange";
arguments["x-message-ttl"] = 3600 * 1000;
arguments["x-expires"] = 7200 * 1000;

// declare the queue
channel.declareQueue("my-queue-name", AMQP::durable + AMQP::autodelete, arguments);
````

PUBLISHING MESSAGES
===================

Publishing messages is easy, and the Channel class has a list of methods that
can all be used for it. The most simple one takes three arguments: the name of the
exchange to publish to, the routing key to use, and the actual message that
you're publishing - all these parameters are standard C++ strings.

More extended versions of the publish() method exist that accept additional
arguments, and that enable you to publish entire Envelope objects, which are
objects that contain the message plus a list of optional meta information like 
the content-type, content-encoding, priority, expire time and more. None of these 
meta fields are interpreted by this library, and also the RabbitMQ ignores most 
of them, but the AMQP protocol defines them, and they are free for you to use. 
For an extensive list of the fields that are supported, take a look at the MetaData.h 
header file (MetaData is the base class for Envelope). You should also check the 
RabbitMQ documentation to find out if an envelope header is interpreted by the 
RabbitMQ server (at the time of this writing, only the expire time is being used).

The following snippet is copied from the Channel.h header file and lists all
available publish() methods. As you can see, you can call the publish() method
in almost any form:

````c++
/**
 *  Publish a message to an exchange
 * 
 *  The following flags can be used
 * 
 *      -   mandatory   if set, an unroutable message will be reported to the channel handler with the onReturned method
 *      -   immediate   if set, a message that could not immediately be consumed is returned to the onReturned method
 * 
 *  If either of the two flags is set, and the message could not immediately
 *  be published, the message is returned by the server to the client. If you
 *  want to catch such returned messages, you need to implement the 
 *  ChannelHandler::onReturned() method.
 * 
 *  @param  exchange    the exchange to publish to
 *  @param  routingkey  the routing key
 *  @param  flags       optional flags (see above)
 *  @param  envelope    the full envelope to send
 *  @param  message     the message to send
 *  @param  size        size of the message
 */
bool publish(const std::string &exchange, const std::string &routingKey, int flags, const AMQP::Envelope &envelope);
bool publish(const std::string &exchange, const std::string &routingKey, const AMQP::Envelope &envelope);
bool publish(const std::string &exchange, const std::string &routingKey, int flags, const std::string &message);
bool publish(const std::string &exchange, const std::string &routingKey, const std::string &message);
bool publish(const std::string &exchange, const std::string &routingKey, int flags, const char *message, size_t size);
bool publish(const std::string &exchange, const std::string &routingKey, const char *message, size_t size);
````

Published messages are normally not confirmed by the server, hence there is no
ChannelHandler::onPublished() method that you can implement to find out if
a message was correctly received by the server. That's by design in the
AMQP protocol, to not unnecessarily slow down message publishing. As long 
as no error is reported via the ChannelHandler::onError() method, you can safely 
assume that your messages were delivered.

If you use the flags parameter to set either the option 'mandatory' or 
'immediate', a message that could not be routed or directly delivered to a consumer 
is sent back to the client, and ends up in the ChannelHandler::onReturned() 
method. At the time of this writing however, the 'immediate' option does not 
seem to be supported by RabbitMQ.


CONSUMING MESSAGES
==================

Fetching messages from RabbitMQ is called consuming, and can be started by calling
the method Channel::consume(). After you've called this method, RabbitMQ starts
delivering messages to you.

Just like the publish() method that we just described, the consume() method also
comes in many forms. The first parameter is always the name of the queue you like
to consume from. The subsequent parameters are an optional consumer tag, flags and
a table with custom arguments. The first additional parameter, the consumer tag,
is nothing more than a string identifier that will be passed with every consumed message.
This can be useful if you call the consume() methods a number of times to consume
from multiple queues, and you would like to know from which consume call the received messages came.

The full documentation from the C++ Channel.h headerfile looks like this:

````c++
/**
 *  Tell the RabbitMQ server that we're ready to consume messages
 * 
 *  After this method is called, RabbitMQ starts delivering messages to the client
 *  application. The consume tag is a string identifier that will be passed to
 *  each received message, so that you can associate incoming messages with a 
 *  consumer. If you do not specify a consumer tag, the server will assign one
 *  for you.
 * 
 *  The following flags are supported:
 * 
 *      -   nolocal             if set, messages published on this channel are not also consumed
 *      -   noack               if set, consumed messages do not have to be acked, this happens automatically
 *      -   exclusive           request exclusive access, only this consumer can access the queue
 *      -   nowait              the server does not have to send a response back that consuming is active
 * 
 *  The method ChannelHandler::onConsumerStarted() will be called when the 
 *  consumer has started (unless the nowait option was set, in which case
 *  no confirmation method is called)
 * 
 *  @param  queue               the queue from which you want to consume
 *  @param  tag                 a consumer tag that will be associated with this consume operation
 *  @param  flags               additional flags
 *  @param  arguments           additional arguments
 *  @return bool
 */
bool consume(const std::string &queue, const std::string &tag, int flags, const AMQP::Table &arguments);
bool consume(const std::string &queue, const std::string &tag, int flags = 0);
bool consume(const std::string &queue, const std::string &tag, const AMQP::Table &arguments);
bool consume(const std::string &queue, int flags, const AMQP::Table &arguments);
bool consume(const std::string &queue, int flags = 0);
bool consume(const std::string &queue, const AMQP::Table &arguments);
````

In your ChannelHandler you can override the onConsumerStarted() method, that will be
first called before any messages are sent to you. Most users choose not to override this
method, because there is not much useful to do in it. After the consumer has started, however,
messages are starting to be sent from RabbitMQ to your client application, and they are all
passed to the ChannelHandler::onReceived() method. This method is thus very important to implement.

````c++
class MyChannelHandler : public AMQP::ChannelHandler
{
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
    virtual void onReceived(AMQP::Channel *channel, const AMQP::Message &message, uint64_t deliveryTag, const std::string &consumerTag, bool redelivered) 
    {
        // @todo
        //  add your own processing
        
        
        // after the message was processed, acknowledge it
        channel->ack(deliveryTag);
    }
}
````

The Message object holds all information of the delivered message: the actual content, 
all meta information from the envelope (in fact, the Message class is derived from the Envelope class),
and even the name of the exchange and the routing key that were used when the message was originally
published. For a full list of all information in the Message class, you best have a look at the
message.h, envelope.h and metadata.h header files.

Another important parameter to the onReceived() method is the deliveryTag parameter. This is a 
unique identifier that you need to acknowledge an incoming message. RabbitMQ only removes the
message after it has been acknowledged, so that if your application crashes while it was busy 
processing the message, the message does not get lost but remains in the queue. But this means that
after you've processed the message, you must inform RabbitMQ about it by calling the Channel:ack()
method. This method is very simple and takes in its simplest form only one parameter: the
deliveryTag of the message.

The consumerTag that you see in the onReceived method() is the same string identifier that was
passed to the Channel::consume() method.

Consuming messages is a continuous process. RabbitMQ keeps sending messages, until you stop
the consumer, which can be done by calling the Channel::cancel() method. If you close the channel,
or the entire TCP connection, consuming also stops.

RabbitMQ throttles the number of messages that are delivered to you, to prevent that your application
is flooded with messages from the queue, and to spread out the messages over multiple consumers.
This is done with a setting called quality-of-service (QOS). The QOS setting is a numeric value which 
holds the number of unacknowledged messages that you are allowed to have. RabbitMQ stops sending 
additional messages when the number of unacknowledges messages has reached this limit, and only 
sends additional messages when an earlier message gets acknowledged. To change the QOS, you can 
simple call Channel::setQos().


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
match the request that we sent before, we do not report an error (this is also 
an issue that only occurs in theory).

It would be nice to have sample implementations for the ConnectionHandler
class that can be directly plugged into libev, libevent and libuv event loops.

For performance reasons, we need to investigate if we can limit the number of times
an incoming or outgoing messages is copied.

