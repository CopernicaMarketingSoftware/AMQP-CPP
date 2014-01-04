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

