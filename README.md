AMQP-CPP
========

AMQP-CPP is a C++ library for communicating with a RabbitMQ message broker. The
library can be used to parse incoming data from a RabbitMQ server, and to
generate frames that can be sent to a RabbitMQ server.

This library has a layered architecture, and allows you - if you like - to 
completely take care of the network layer. If you want to set up and manage the 
network connections yourself, the AMQP-CPP library will not make a connection to
RabbitMQ by itself, nor will it create sockets and/or perform IO operations. As 
a user of this library, you create the socket connection and implement a certain 
interface that you pass to the AMQP-CPP library and that the library will use 
for IO operations.

Intercepting this network layer is however optional, the AMQP-CPP library also 
comes with a predefined Tcp module that can be used if you trust the AMQP library 
to take care of the network handling. In that case, the AMQP-CPP library does
all the system calls to set up network connections and send and receive the data.

This layered architecture makes the library extremely flexible and portable: it 
does not necessarily rely on operating system specific IO calls, and can be 
easily integrated into any kind of event loop. If you want to implement the AMQP
protocol on top of some [unusual other communication layer](https://tools.ietf.org/html/rfc1149), 
this library can be used for that - but if you want to use it with regular TCP 
connections, setting it up is just as easy. 

AMQP-CPP is fully asynchronous and does not do any blocking (system) calls, so 
it can be used in high performance applications without the need for threads.

The AMQP-CPP library uses C++11 features, so if you intend use it, please make 
sure that your compiler is up-to-date and supports C++11.

**Note for the reader:** This readme file has a peculiar structure. We start 
explaining the pure and hard core low level interface in which you have to
take care of opening socket connections yourself. In reality, you probably want
to use the simpler TCP interface that is being described later on.


ABOUT
=====

This library is created and maintained by Copernica (www.copernica.com), and is
used inside the MailerQ (www.mailerq.com), Yothalot (www.yothalot.com) and 
AMQPipe (www.amqpipe.com) applications. MailerQ is a tool for sending large 
volumes of email, using AMQP message queues, Yothalot is a big data processing
map/reduce framework and AMQPipe is a tool for high-speed processing messages 
between AMQP pipes

Do you appreciate our work and are you looking for high quality email solutions? 
Then check out our other commercial and open source solutions:

* Copernica Marketing Suite (www.copernica.com)
* MailerQ on-premise MTA (www.mailerq.com)
* AMQPipe (www.amqpipe.com)
* Responsive Email web service (www.responsiveemail.com)
* SMTPeter cloud based SMTP server (www.smtpeter.com)
* PHP-CPP bridge between PHP and C++ (www.php-cpp.com)
* PHP-JS bridge between PHP and Javascript (www.php-js.com)
* Yothalot big data processor (www.yothalot.com)


INSTALLING
==========

If you are on some kind of Linux environment, installing the library is as easy
as running `make` and `make install`. This will install the full version of
the AMQP-CPP, including the system specific TCP module. If you do not need the
additional TCP module (because you take care of handling the network stuff 
yourself), you can also compile a pure form of the library. Use `make pure` 
and `make install` for that.

For users on a non-Linux environment: this library is known to work on your
environment too (after all, it does not do any operating specific system calls), 
but it might take some extra effort to get your compiler to compile it. Please 
send in your pull requests once you have it running, so that others can benefit 
from your experiences.

When you compile an application that uses the AMQP-CPP library, do not
forget to link with the library. For gcc and clang the linker flag is -lamqpcpp.
If you use the fullblown version of AMQP-CPP (with the TCP module), you also 
need to pass a -lpthread linker flag, because the TCP module uses a thread 
for running an asynchronous and non-blocking DNS hostname lookup.


HOW TO USE AMQP-CPP
===================

As we mentioned above, the library can be used in a network-agnostic fashion.
It then does not do any IO by itself, and you need to pass an object to the 
library that the library can use for IO. So, before you start using the
library, you first you need to create a class that extends from the 
ConnectionHandler base class. This is a class with a number of methods that are 
called by the library every time it wants to send out data, or when it needs to 
inform you that an error occured.

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
    virtual void onError(AMQP::Connection *connection, const char *message)
    {
        // @todo
        //  add your own implementation, for example by reporting the error
        //  to the user of your program, log the error, and destruct the
        //  connection object because it is no longer in a usable state
    }

    /**
     *  Method that is called when the connection was closed. This is the
     *  counter part of a call to Connection::close() and it confirms that the
     *  connection was correctly closed.
     *
     *  @param  connection      The connection that was closed and that is now unusable
     */
    virtual void onClosed(AMQP::Connection *connection) {}


};
````
After you've implemented the ConnectionHandler class, you can start using
the library by creating a Connection object, and one or more Channel objects:

````c++
// create an instance of your own connection handler
MyConnectionHandler myHandler;

// create a AMQP connection object
AMQP::Connection connection(&myHandler, AMQP::Login("guest","guest"), "/");

// and create a channel
AMQP::Channel channel(&connection);

// use the channel object to call the AMQP method you like
channel.declareExchange("my-exchange", AMQP::fanout);
channel.declareQueue("my-queue");
channel.bindQueue("my-exchange", "my-queue", "my-routing-key");
````

A number of remarks about the example above. First you may have noticed that we've
created all objects on the stack. You are of course also free to create them
on the heap with the C++ operator 'new'. That works just as well, and is in real
life code probably more useful as you normally want to keep your handlers, connection
and channel objects around for a longer time.

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
implement that method. Inside your ConnectionHandler::onData() method, you can for
example call the "send()" or "write()" system call to send out the data to
the RabbitMQ server. But what about data in the other direction? How does the
library receive data back from RabbitMQ?

In this raw setup, the AMQP-CPP library does not do any IO by itself and it is 
therefore also not possible for the library to receive data from a 
socket. It is again up to you to do this. If, for example, you notice in your 
event loop that the socket that is connected with the RabbitMQ server becomes 
readable, you should read out that socket (for example by using the recv() system 
call), and pass the received bytes to the AMQP-CPP library. This is done by 
calling the parse() method in the Connection object.

The Connection::parse() method gets two parameters, a pointer to a buffer of
data that you just read from the socket, and a parameter that holds the size of 
this buffer. The code snippet below comes from the Connection.h C++ header file.

````c++
/**
 *  Parse data that was recevied from RabbitMQ
 *
 *  Every time that data comes in from RabbitMQ, you should call this method to parse
 *  the incoming data, and let it handle by the AMQP-CPP library. This method returns
 *  the number of bytes that were processed.
 *
 *  If not all bytes could be processed because it only contained a partial frame,
 *  you should call this same method later on when more data is available. The
 *  AMQP-CPP library does not do any buffering, so it is up to the caller to ensure
 *  that the old data is also passed in that later call.
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

You should do all the book keeping for the buffer yourselves. If you for example
call the Connection::parse() method with a buffer of 100 bytes, and the method
returns that only 60 bytes were processed, you should later call the method again,
with a buffer filled with the remaining 40 bytes. If the method returns 0, you should
make a new call to parse() when more data is available, with a buffer that contains
both the old data, and the new data.

To optimize your calls to the parse() method, you _could_ use the Connection::expected()
and Connection::maxFrame() methods. The expected() method returns the number of bytes
that the library prefers to receive next. It is pointless to call the parse() method
with a smaller buffer, and it is best to call the method with a buffer of exactly this 
size. The maxFrame() returns the max frame size for AMQP messages. If you read your 
messages into a reusable buffer, you could allocate this buffer up to this size, so that 
you never will have to reallocate.


TCP CONNECTIONS
===============

Although the AMQP-CPP library gives you extreme flexibility by letting you setup
your own network connections, the reality is that most if not all AMQP connections 
use the TCP protocol. To help you out, the library therefore also comes with a 
TCP module that takes care of setting up the network connections, and sending
and receiving the data. 

If you want to use this TCP module, you should not use the AMQP::Connection
and AMQP::Channel classes that you saw above, but the alternative AMQP::TcpConnection
and AMQP::TcpChannel classes instead. You also do not have to create your own class
that implements the "AMQP::ConnectionHandler" interface - but a class that inherits 
from "AMQP::TcpHandler" instead. You especially need to implement the "monitor()" 
method in that class, as that is needed by the AMQP-CPP library to interact with 
the main event loop:

````c++
#include <amqpcpp.h>

class MyTcpHandler : public AMQP::TcpHandler
{
    /**
     *  Method that is called by the AMQP library when the login attempt
     *  succeeded. After this method has been called, the connection is ready
     *  to use.
     *  @param  connection      The connection that can now be used
     */
    virtual void onConnected(AMQP::TcpConnection *connection)
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
    virtual void onError(AMQP::TcpConnection *connection, const char *message)
    {
        // @todo
        //  add your own implementation, for example by reporting the error
        //  to the user of your program, log the error, and destruct the
        //  connection object because it is no longer in a usable state
    }

    /**
     *  Method that is called when the connection was closed. This is the
     *  counter part of a call to Connection::close() and it confirms that the
     *  connection was correctly closed.
     *
     *  @param  connection      The connection that was closed and that is now unusable
     */
    virtual void onClosed(AMQP::TcpConnection *connection) {}

    /**
     *  Method that is called by the AMQP-CPP library when it wants to interact
     *  with the main event loop. The AMQP-CPP library is completely non-blocking,
     *  and only make "write()" or "read()" system calls when it knows in advance
     *  that these calls will not block. To register a filedescriptor in the
     *  event loop, it calls this "monitor()" method with a filedescriptor and
     *  flags telling whether the filedescriptor should be checked for readability
     *  or writability.
     *
     *  @param  connection      The connection that wants to interact with the event loop
     *  @param  fd              The filedescriptor that should be checked
     *  @param  flags           Bitwise or of AMQP::readable and/or AMQP::writable
     */
    virtual void monitor(AMQP::TcpConnection *connection, int fd, int flags)
    {
        // @todo
        //  add your own implementation, for example by adding the file
        //  descriptor to the main application event loop (like the select() or
        //  poll() loop). When the event loop reports that the descriptor becomes
        //  readable and/or writable, it is up to you to inform the AMQP-CPP
        //  library that the filedescriptor is active by calling the 
        //  connection->process(fd, flags) method.
    }
};
````

The "monitor()" method can be used to integrate the AMQP filedescriptors in your
application's event loop. For some popular event loops (libev, libevent), we have
already added example handler objects (see the next section for that).

Using the TCP module of the AMQP-CPP library is easier than using the 
raw AMQP::Connection and AMQP::Channel objects, because you do not have to 
create the sockets and connections yourself, and you also do not have to take
care of buffering network data.

The example that we gave above, looks slightly different if you make use of
the TCP module:

````c++
// create an instance of your own tcp handler
MyTcpHandler myHandler;

// address of the server
AMQP::Address address("amqp://guest:guest@localhost/vhost");

// create a AMQP connection object
AMQP::TcpConnection connection(&myHandler, address);

// and create a channel
AMQP::TcpChannel channel(&connection);

// use the channel object to call the AMQP method you like
channel.declareExchange("my-exchange", AMQP::fanout);
channel.declareQueue("my-queue");
channel.bindQueue("my-exchange", "my-queue", "my-routing-key");
````

EXISTING EVENT LOOPS
====================

Both the pure AMQP::Connection as well as the easier AMQP::TcpConnection class
allow you to integrate AMQP-CPP in your own event loop. Whether you take care
of running the event loop yourself (for example by using the select() system 
call), or if you use an existing library for it (like libevent, libev or libuv), 
you can implement the "monitor()" method to watch the file descriptors and 
hand over control back to AMQP-CPP when one of the sockets become active.

For libev and libevent users, we have even implemented an example implementation,
so that you do not even have to do this. Instead of implementing the monitor() method
yourself, you can use the AMQP::LibEvHandler or AMQP:LibEventHandler classes instead:

````c++
#include <ev.h>
#include <amqpcpp.h>
#include <amqpcpp/libev.h>

int main()
{
    // access to the event loop
    auto *loop = EV_DEFAULT;
    
    // handler for libev (so we don't have to implement AMQP::TcpHandler!)
    AMQP::LibEvHandler handler(loop);
    
    // make a connection
    AMQP::TcpConnection connection(&handler, AMQP::Address("amqp://localhost/"));
    
    // we need a channel too
    AMQP::TcpChannel channel(&connection);
    
    // create a temporary queue
    channel.declareQueue(AMQP::exclusive).onSuccess([&connection](const std::string &name, uint32_t messagecount, uint32_t consumercount) {
        
        // report the name of the temporary queue
        std::cout << "declared queue " << name << std::endl;
        
        // now we can close the connection
        connection.close();
    });
    
    // run the loop
    ev_run(loop, 0);

    // done
    return 0;
}
````

The AMQP::LibEvHandler and AMQP::LibEventHandler classes are extended AMQP::TcpHandler
classes, with an implementation of the monitor() method that simply adds the
filedescriptor to the event loop. If you use this class however, it is recommended not to
instantiate it directly (like we did in the example), but to create your own
"MyHandler" class that extends from it, and in which you also implement the 
onError() method to report possible connection errors to your end users.

Currently, we have only added such an example TcpHandler implementation for libev and
libevent. For other event loops (like libuv and boost asio) we do not yet have
such examples.


CHANNELS
========

In the above example we created a channel object. A channel is a sort of virtual 
connection, and it is possible to create many channels that all use
the same connection.

AMQP instructions are always sent over a channel, so before you can send the first
command to the RabbitMQ server, you first need a channel object. The channel
object has many methods to send instructions to the RabbitMQ server. It for
example has methods to declare queues and exchanges, to bind and unbind them,
and to publish and consume messages. You can best take a look at the channel.h
C++ header file for a list of all available methods. Every method in it is well
documented.

All operations that you can perform on a channel are non-blocking. This means
that it is not possible for a method (like Channel::declareExchange()) to
immediately return 'true' or 'false'. Instead, almost every method of the Channel 
class returns an instance of the 'Deferred' class. This 'Deferred' object can be 
used to install handlers that will be called in case of success or failure.

For example, if you call the channel.declareExchange() method, the AMQP-CPP library
will send a message to the RabbitMQ message broker to ask it to declare the
queue. However, because all operations in the library are asynchronous, the
declareExchange() method can not return 'true' or 'false' to inform you whether
the operation was succesful or not. Only after a while, after the instruction
has reached the RabbitMQ server, and the confirmation from the server has been
sent back to the client, the library can report the result of the declareExchange()
call.

To prevent any blocking calls, the channel.declareExchange() method returns a
'Deferred' result object, on which you can set callback functions that will be
called when the operation succeeds or fails.

````c++
// create a channel (or use TcpChannel if you're using the Tcp module)
Channel myChannel(&connection);

// declare an exchange, and install callbacks for success and failure
myChannel.declareExchange("my-exchange")

    .onSuccess([]() {
        // by now the exchange is created
    })

    .onError([](const char *message) {
        // something went wrong creating the exchange
    });
````

As you can see in the above example, we call the declareExchange() method, and
treat its return value as an object, on which we immediately install a lambda
callback function to handle success, and to handle failure.

Installing the callback methods is optional. If you're not interested in the
result of an operation, you do not have to install a callback for it. Next
to the onSuccess() and onError() callbacks that can be installed, you can also
install a onFinalize() method that gets called directly after the onSuccess()
and onError() methods, and that can be used to set a callback that should
run in either case: when the operation succeeds or when it fails.

The signature for the onError() method is always the same: it gets one parameter
with a human readable error message. The onSuccess() function has a different
signature depending on the method that you call. Most onSuccess() functions
(like the one we showed for the declareExchange() method) do not get any
parameters at all. Some specific onSuccess callbacks receive extra parameters
with additional information.


CHANNEL CALLBACKS
=================

As explained, most channel methods return a 'Deferred' object on which you can
install callbacks using the Deferred::onError() and Deferred::onSuccess() methods.

The callbacks that you install on a Deferred object, only apply to one specific
operation. If you want to install a generic error callback for the entire channel,
you can so so by using the Channel::onError() method. Next to the Channel::onError()
method, you can also install a callback to be notified when the channel is ready
for sending the first instruction to RabbitMQ.

````c++
// create a channel (or use TcpChannel if you use the Tcp module)
Channel myChannel(&connection);

// install a generic channel-error handler that will be called for every
// error that occurs on the channel
myChannel.onError([](const char *message) {

    // report error
    std::cout << "channel error: " << message << std::endl;
});

// install a generic callback that will be called when the channel is ready
// for sending the first instruction
myChannel.onReady([]() {

    // send the first instructions (like publishing messages)
});
````

In theory, you should wait for the onReady() callback to be called before you
send any other instructions over the channel. In practice however, the AMQP library
caches all instructions that were sent too early, so that you can use the
channel object right after it was constructed.


CHANNEL ERRORS
==============

It is important to realize that any error that occurs on a channel, 
invalidates the entire channel, including all subsequent instructions that
were already sent over it. This means that if you call multiple methods in a row,
and the first method fails, all subsequent methods will not be executed either:

````c++
Channel myChannel(&connection);
myChannel.declareQueue("my-queue");
myChannel.declareExchange("my-exchange");
````

If the first declareQueue() call fails in the example above, the second
myChannel.declareExchange() method will not be executed, even when this
second instruction was already sent to the server. The second instruction will be
ignored by the RabbitMQ server because the channel was already in an invalid
state after the first failure.

You can overcome this by using multiple channels:

````c++
Channel channel1(&connection);
Channel channel2(&connection);
channel1.declareQueue("my-queue");
channel2.declareExchange("my-exchange");
````

Now, if an error occurs with declaring the queue, it will not have consequences
for the other call. But this comes at a small price: setting up the extra channel
requires and extra instruction to be sent to the RabbitMQ server, so some extra
bytes are sent over the network, and some additional resources in both the client
application and the RabbitMQ server are used (although this is all very limited).

If possible, it is best to make use of this feature. For example, if you have an important AMQP 
connection that you use for consuming messages, and at the same time you want
to send another instruction to RabbitMQ (like declaring a temporary queue), it is
best to set up a new channel for this 'declare' instruction. If the declare fails,
it will not stop the consumer, because it was sent over a different channel.

The AMQP-CPP library allows you to create channels on the stack. It is not
a problem if a channel object gets destructed before the instruction was received by
the RabbitMQ server:

````c++
void myDeclareMethod(AMQP::Connection *connection)
{
    // create temporary channel to declare a queue
    AMQP::Channel channel(connection);
    
    // declare the queue (the channel object is destructed before the
    // instruction reaches the server, but the AMQP-CPP library can deal
    // with this)
    channel.declareQueue("my-new-queue");
}
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
DeferredQueue &declareQueue(const std::string &name, int flags, const Table &arguments);
DeferredQueue &declareQueue(const std::string &name, const Table &arguments);
DeferredQueue &declareQueue(const std::string &name, int flags = 0);
DeferredQueue &declareQueue(int flags, const Table &arguments);
DeferredQueue &declareQueue(const Table &arguments);
DeferredQueue &declareQueue(int flags = 0);
````

As you can see, the method comes in many forms, and it is up to you to choose
the one that is most appropriate. We now take a look at the most complete
one, the method with three parameters.

All above methods returns a 'DeferredQueue' object. The DeferredQueue class
extends from the AMQP::Deferred class and allows you to install a more powerful
onSuccess() callback function. The 'onSuccess' method for the declareQueue()
function gets three arguments:

````c++
// create a custom callback
auto callback = [](const std::string &name, int msgcount, int consumercount) {

    // @todo add your own implementation

}

// declare the queue, and install the callback that is called on success
channel.declareQueue("myQueue").onSuccess(callback);
````

Just like many others methods in the Channel class, the declareQueue() method
accepts an integer parameter named 'flags'. This is a variable in which you can
set method-specific options, by summing up all the options that are described in
the documentation above the method. If you for example want to create a durable,
auto-deleted queue, you can pass in the value AMQP::durable + AMQP::autodelete.

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
arguments, and that enable you to publish entire Envelope objects. An envelope
is an object that contains the message plus a list of optional meta properties like
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
 *      -   mandatory   if set, an unroutable message will be sent back to
 *                      the client (currently not supported)
 *
 *      -   immediate   if set, a message that could not immediately be consumed
 *                      is returned to the client (currently not supported)
 *
 *  If either of the two flags is set, and the message could not immediately
 *  be published, the message is returned by the server to the client. However,
 *  at this moment in time, the AMQP-CPP library does not support catching
 *  such returned messages.
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

Published messages are normally not confirmed by the server, and the RabbitMQ
will not send a report back to inform you whether the message was succesfully
published or not. Therefore the publish method does not return a Deferred
object.

As long as no error is reported via the Channel::onError() method, you can safely
assume that your messages were delivered.

This can of course be a problem when you are publishing many messages. If you get
an error halfway through there is no way to know for sure how many messages made
it to the broker and how many should be republished. If this is important, you can
wrap the publish commands inside a transaction. In this case, if an error occurs,
the transaction is automatically rolled back by RabbitMQ and none of the messages
are actually published.

````c++
// start a transaction
channel.startTransaction();

// publish a number of messages
channel.publish("my-exchange", "my-key", "my first message");
channel.publish("my-exchange", "my-key", "another message");

// commit the transactions, and set up callbacks that are called when
// the transaction was successful or not
channel.commitTransaction()
    .onSuccess([]() {
        // all messages were successfully published
    })
    .onError([]() {
        // none of the messages were published
        // now we have to do it all over again
    });
````

Note that AMQP transactions are not as powerful as transactions that are 
knows in the database world. It is not possible to wrap all sort of 
operations in a transaction, they are only meaningful for publishing
and consuming.


CONSUMING MESSAGES
==================

Fetching messages from RabbitMQ is called consuming, and can be started by calling
the method Channel::consume(). After you've called this method, RabbitMQ starts
delivering messages to you.

Just like the publish() method that we just described, the consume() method also
comes in many forms. The first parameter is always the name of the queue you like
to consume from. The subsequent parameters are an optional consumer tag, flags and
a table with custom arguments. The first additional parameter, the consumer tag,
is nothing more than a string identifier that you can use when you want to stop
consuming.

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
 *      -   nolocal             if set, messages published on this channel are
 *                              not also consumed
 *
 *      -   noack               if set, consumed messages do not have to be acked,
 *                              this happens automatically
 *
 *      -   exclusive           request exclusive access, only this consumer can
 *                              access the queue
 *
 *  The callback registered with DeferredConsumer::onSuccess() will be called when the
 *  consumer has started.
 *
 *  @param  queue               the queue from which you want to consume
 *  @param  tag                 a consumer tag that will be associated with this consume operation
 *  @param  flags               additional flags
 *  @param  arguments           additional arguments
 *  @return bool
 */
DeferredConsumer &consume(const std::string &queue, const std::string &tag, int flags, const AMQP::Table &arguments);
DeferredConsumer &consume(const std::string &queue, const std::string &tag, int flags = 0);
DeferredConsumer &consume(const std::string &queue, const std::string &tag, const AMQP::Table &arguments);
DeferredConsumer &consume(const std::string &queue, int flags, const AMQP::Table &arguments);
DeferredConsumer &consume(const std::string &queue, int flags = 0);
DeferredConsumer &consume(const std::string &queue, const AMQP::Table &arguments);
````

As you can see, the consume method returns a DeferredConsumer. This object is a
regular Deferred, with  additions. The onSuccess() method of a
DeferredConsumer is slightly different than the onSuccess() method of a regular
Deferred object: one extra parameter will be supplied to your callback function
with the consumer tag.

The onSuccess() callback will be called when the consume operation _has started_,
but not when messages are actually consumed. For this you will have to install
a different callback, using the onReceived() method.

````c++
// callback function that is called when the consume operation starts
auto startCb = [](const std::string &consumertag) {

    std::cout << "consume operation started" << std::endl;
};

// callback function that is called when the consume operation failed
auto errorCb = [](const char *message) {

    std::cout << "consume operation failed" << std::endl;
}

// callback operation when a message was received
auto messageCb = [&channel](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {

    std::cout << "message received" << std::endl;

    // acknowledge the message
    channel.ack(deliveryTag);
}

// start consuming from the queue, and install the callbacks
channel.consume("my-queue")
    .onReceived(messageCb)
    .onSuccess(startCb)
    .onError(errorCb);

````

The Message object holds all information of the delivered message: the actual
content, all meta information from the envelope (in fact, the Message class is
derived from the Envelope class), and even the name of the exchange and the
routing key that were used when the message was originally published. For a full
list of all information in the Message class, you best have a look at the
message.h, envelope.h and metadata.h header files.

Another important parameter to the onReceived() method is the deliveryTag parameter.
This is a unique identifier that you need to acknowledge an incoming message.
RabbitMQ only removes the message after it has been acknowledged, so that if your
application crashes while it was busy processing the message, the message does
not get lost but remains in the queue. But this means that after you've processed
the message, you must inform RabbitMQ about it by calling the Channel:ack() method.
This method is very simple and takes in its simplest form only one parameter: the
deliveryTag of the message.

Consuming messages is a continuous process. RabbitMQ keeps sending messages, until
you stop the consumer, which can be done by calling the Channel::cancel() method.
If you close the channel, or the entire TCP connection, consuming also stops.

RabbitMQ throttles the number of messages that are delivered to you, to prevent
that your application is flooded with messages from the queue, and to spread out
the messages over multiple consumers. This is done with a setting called
quality-of-service (QOS). The QOS setting is a numeric value which holds the number
of unacknowledged messages that you are allowed to have. RabbitMQ stops sending
additional messages when the number of unacknowledges messages has reached this
limit, and only sends additional messages when an earlier message gets acknowledged.
To change the QOS, you can simple call Channel::setQos().


WORK IN PROGRESS
================

Almost all AMQP features have been implemented. But the following things might
need additional attention:

    -   ability to set up secure connections (or is this fully done on the IO level)
    -   login with other protocols than login/password
    -   publish confirms
    -   returned messages

We also need to add more safety checks so that strange or invalid data from
RabbitMQ does not break the library (although in reality RabbitMQ only sends
valid data). Also, when we now receive an answer from RabbitMQ that does not
match the request that we sent before, we do not report an error (this is also
an issue that only occurs in theory).

It would be nice to have sample implementations for the ConnectionHandler
class that can be directly plugged into libev, libevent and libuv event loops.

For performance reasons, we need to investigate if we can limit the number of times
an incoming or outgoing messages is copied.

