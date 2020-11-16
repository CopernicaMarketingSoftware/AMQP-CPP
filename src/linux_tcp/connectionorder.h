/**
 *  ConnectionOrder.h
 *  
 *  Class that give info on how we want to sellect the connection from a list of IPs
 * 
 *  @author Aljar Meesters <aljar.meesters@copernica.com>
 *  @copyright 2020 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Setup namespace
 */
namespace AMQP
{

/**
 *  Class implementation
 */
class ConnectionOrder
{
public:
    /**
     *  Enum class holding the orders we support
     *  - standard:   what is used by getaddrinfo, which is proximity based
     *  - reverse:    reverse the standard order
     *  - random:     random order
     *  - ascending:  try the smallest IP address first
     *  - descending: try the largest IP address first
     *  @var enum Order
     */
    enum class Order { standard, reverse, random, ascending, descending};

private: 
    /**
     *  The order for the connection
     *  @var Order
     */
    Order _order;

public:
    /**
     *  Constructor
     *  @var  order
     */
    ConnectionOrder(const char *order) : _order(Order::standard)
    {
        // Set the orders based on the string
        if      (order == nullptr) {} // first check if the order is not null
        else if (strcmp(order, "random") == 0) _order = Order::random;
        else if (strcmp(order, "ascending") == 0 || strcmp(order, "asc") == 0) _order = Order::ascending;
        else if (strcmp(order, "descending") == 0 || strcmp(order, "desc") == 0 ) _order = Order::descending;
    }
    
    /**
     *  Destructor
     */
    virtual ~ConnectionOrder() = default;

    /**
     *  Get the order
     *  @return Order
     */
    const Order &order() const
    {
        return _order;
    }
};

/**
 *  End of namespace
 */
}
