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
event loop. It also does not do any blocking system calls, so it can be used
in high performance applications without the need for threads.


ABOUT
=====

This library is created and maintained by Copernica (www.copernica.com), and is 
used inside the MailerQ (www.mailerq.com) application, MailerQ is a tool for 
sending large volumes of email, using AMQP message queues.


HOW TO USE
==========

First you need to implement the ConnectionHandler interface. This is a class
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
on the heap with the C++ operator 'new'. That works just as good.

But more importantly, you can see in the example above that we have created the 
channel object directly after we made the connection object, and we also 
started declaring exchanges and queues right away. It would have been better if
we had waited for the connection to be ready, and create the channel object
inside the onConnected() method in the MyConnectionHandler class. But this is 
not strictly necessary. The methods that are called before the connection is
ready are cached by the AMQP library and will be executed the moment the 
connection becomes ready for use.

As we've explained above, the AMQP library does not do any IO by itself and when it
needs to send data to RabbitMQ, it will call the onData() method in the handler
object. It is of course also not possible for the library to receive data from 
the server. It is again up to you to to this. If, for example, you notice in your 
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

The channel object has many methods to declare queues and exchanges, to bind
and unbind them, to publish and consume messages - and more. You can best take a 
look in the channel.h C++ header file for a list of all available methods. Every 
method in it is well documented.

The constructor of the Channel object gets two parameters: the connection object,
and a pointer to a ChannelHandler object. In the first example that we gave we have 
not yet used this ChannelHandler object. However, in normal circumstances when you 
construct a Channel object, you should also pass a pointer to a ChannelHandler object. 

Just like the ConnectionHandler class, the ChannelHandler class is a base class that 
you should extend and override the virtual methods that you need. The AMQP library 
will call these methods to inform you that an operation has succeeded or has failed.
For example, if you call the channel.declareQueue() method, the AMQP library will
send a message to the RabbitMQ message broker to ask it to declare the
queue, and return true to indicate that the message has been sent. However, this
does not mean that the queue has succesfully been declared. This is only known
after the server has sent back a message to the client to report whether the
queue was succesfully created or not. When this answer is received, the AMQP library 
will call the method ChannelHandler::onQueueDeclared() method - which you can
override in your ChannelHandler object.

All methods in the base ChannelHandler class have a default empty implementation,
so you do not have to implement all of them - only the ones that you are interested
in.

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
     *  Method that is called when a queue has been declared
     *  @param  channel         the channel via which the queue was declared
     *  @param  name            name of the queue
     *  @param  messageCount    number of messages in queue
     *  @param  consumerCount   number of active consumers
     */
    virtual void onQueueDeclared(AMQP::Channel *channel, const std::string &name, uint32_t messageCount, uint32_t consumerCount)
    {
        // @todo
        //  do something with the information that cam back, or start using the queue
    }
};
````

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

The declareQueue() method also accepts a arguments parameter, which is of type
Table. The Table object can be used as an associative array to send additional
options to RabbitMQ, that are often custom RabbitMQ extensions to the AMQP 
standard:

````c++
// custom options that are passed to the declareQueue call
Table arguments;
arguments["x-message-ttl"] = 3600 * 1000;
arguments["x-expires"] = 7200 * 1000;

// declare the queue
channel.declareQueue("my-queue-name", AMQP::durable + AMQP::autodelete, arguments);
````

WORK IN PROGRESS
================

It is not yet possible to publish messages, and consume messages with this library.
These features will soon be added.

