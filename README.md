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
event loop.


HOW TO USE
==========

First you need to implement the ConnectionHandler interface. This is a class
with a number of methods that are called by the library every time it wants
to send out data, or when it needs to inform you that an error occured.

````c++
#include <amqp.h>

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

A number of remarks about the above example. First you may have noticed that we've
created all objects on the stack. You are of course also free to create them
on the heap with the C++ operator 'new'. That works just as good.

But more importantly, you see that in the example above we have created the 
channel object directly after we created the connection object, and we did also 
start declaring exchanges and queues right away. It would have been better to 
first wait for the connection to be ready (and the onConnected() method is called
in your handler object), before you create channel objects and start calling
methods. But if you insist, you do not have to wait, and you are free to call 
the additional methods right away.

As we've explained above, the AMQP library does not do any IO by itself and when it
needs to send data to RabbitMQ, it will call the onData() method in your handler
object. But it is of course also not possible for the library to receive data from 
the server. It is again up to you to to this. If, for example, you notice in your 
event loop that the socket that is connected with the RabbitMQ server becomes 
readable, you should read out that data (for example by calling the recv() system 
call), and pass the received bytes to the AMQP library. This can be done by
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


