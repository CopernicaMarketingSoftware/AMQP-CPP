#pragma once
/**
 *  Message.h
 *
 *  An incoming message has the same sort of information as an outgoing
 *  message, plus some additional information.
 *
 *  Message objects can not be constructed by end users, they are only constructed
 *  by the AMQP library, and passed to user callbacks.
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class Message : public Envelope
{
protected:
    /**
     *  The exchange to which it was originally published
     *  @var    string
     */
    std::string _exchange;

    /**
     *  The routing key that was originally used
     *  @var    string
     */
    std::string _routingKey;

protected:
    /**
     *  The constructor is protected to ensure that endusers can not
     *  instantiate a message
     *  @param  exchange
     *  @param  routingKey
     */
    Message(const std::string &exchange, const std::string &routingKey) :
        Envelope(nullptr, 0), _exchange(exchange), _routingKey(routingKey)
    {}

public:

    /**
     *  Copy constructor
     *
     *  @param  message the message to copy
     */
    Message(const Message &message) :
        Envelope(message),
        _exchange(message._exchange),
        _routingKey(message._routingKey)
    {}

    /**
     *  Move constructor
     *
     *  @param  message the message to move
     */
    Message(Message &&message) :
        Envelope(std::move(message)),
        _exchange(std::move(message._exchange)),
        _routingKey(std::move(message._routingKey))
    {}

    /**
     *  Destructor
     */
    virtual ~Message() {}

    /**
     *  Assignment operator
     *
     *  @param  message the message to copy
     *  @return same object for chaining
     */
    Message &operator=(const Message &message)
    {
        // call the base assignment
        Envelope::operator=(message);

        // move the exchange and routing key
        _exchange   = message._exchange;
        _routingKey = message._routingKey;

        // allow chaining
        return *this;
    }

    /**
     *  Move assignment operator
     *
     *  @param  message the message to move
     *  @return same object for chaining
     */
    Message &operator=(Message &&message)
    {
        // call the base assignment
        Envelope::operator=(std::move(message));

        // move the exchange and routing key
        _exchange   = std::move(message._exchange);
        _routingKey = std::move(message._routingKey);

        // allow chaining
        return *this;
    }

    /**
     *  The exchange to which it was originally published
     *  @var    string
     */
    const std::string &exchange() const
    {
        return _exchange;
    }

    /**
     *  The routing key that was originally used
     *  @var    string
     */
    const std::string &routingKey() const
    {
        return _routingKey;
    }
};

/**
 *  End of namespace
 */
}

