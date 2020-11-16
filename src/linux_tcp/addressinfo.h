/**
 *  AddressInfo.h
 *
 *  Utility wrapper around "getAddressInfo()"
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Dependencies
 */
#include <random>
#include "connectionorder.h"

/**
 *  Include guard
 */
namespace AMQP {

/**
 *  Class definition
 */
class AddressInfo
{
private:
    /**
     *  The addresses
     *  @var struct AddressInfo
     */
    struct addrinfo *_info = nullptr;
    
    /**
     *  Vector of addrinfo pointers
     *  @var std::vector<struct addrinfo *>
     */
    std::vector<struct addrinfo *> _v;

    /**
     *  Helper function to order the vector of addrinfo based on the ordering received
     * This may be useful since getaddrinfo is sorting the addresses on proximity
     * (e.g. https://lists.debian.org/debian-glibc/2007/09/msg00347.html),
     *  @param order
     */
    void reorder(const ConnectionOrder &order)
    {    
        // witch on order
        switch (order.order()) {
        case ConnectionOrder::Order::random:        shuffle();         break;
        case ConnectionOrder::Order::ascending:     sort();            break;
        case ConnectionOrder::Order::descending:    sort(); reverse(); break;
        case ConnectionOrder::Order::reverse:       reverse();         break;
        default:                                                       break;
        }
    }

public:
    /**
     *  Constructor
     *  @param  hostname
     *  @param  port
     *  @param  order
     */
    AddressInfo(const char *hostname, uint16_t port, const ConnectionOrder &order)
    {
        // store portnumber in buffer
        auto portnumber = std::to_string(port);
        
        // info about the lookup
        struct addrinfo hints;
        
        // set everything to zero
        memset(&hints, 0, sizeof(struct addrinfo));
        
        // set hints
        hints.ai_family = AF_UNSPEC;        // allow IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM;    // datagram socket/
        
        // get address of the server
        auto code = getaddrinfo(hostname, portnumber.data(), &hints, &_info);
        
        // was there an error
        if (code != 0) throw std::runtime_error(gai_strerror(code));
        
        // keep looping
        for (auto *current = _info; current; current = current->ai_next)
        {
            // store in vector
            _v.push_back(current);
        }

        // Order the vector based on the provided ordering
        reorder(order);
    }

    /**
     *  Destructor
     */
    virtual ~AddressInfo()
    {
        // free address info
        freeaddrinfo(_info);
    }
    
    /**
     *  Shuffle the addresses
     */
    void shuffle()
    {
        // create a random device for the seed of the random number generator
        std::random_device rd;

        // Create the generator
        std::mt19937 gen(rd());

        // shuffle the vector.
        std::shuffle(_v.begin(), _v.end(), gen);
    }
    
    /**
     *  Order the addresses based on IP
     */
    void sort()
    {
        // sort the addresses
        std::sort(_v.begin(), _v.end(), [](const struct addrinfo *v1, const struct addrinfo *v2) -> bool {
            
            // check the IP versions (they must be identical to be comparable)
            if (v1->ai_family != v2->ai_family) return v1->ai_family < v2->ai_family;
            
            // should we compare ipv4 or ipv6?
            switch (v1->ai_family) {
            case AF_INET: {
                // ugly cast
                auto *a1 = (struct sockaddr_in *)(v1->ai_addr);
                auto *a2 = (struct sockaddr_in *)(v2->ai_addr);
                
                // do the comparison for the addresses
                return a1->sin_addr.s_addr < a2->sin_addr.s_addr;
            }
            case AF_INET6: {
                // ugly cast
                auto *a1 = (struct sockaddr_in6 *)(v1->ai_addr);
                auto *a2 = (struct sockaddr_in6 *)(v2->ai_addr);
                
                // do the comparison
                return memcmp(&a1->sin6_addr, &a2->sin6_addr, sizeof(in6_addr)) < 0;
            }
            default:
                return false;
            }
        });
    }
    
    /**
     *  Reverse the order
     */
    void reverse()
    {
        // reverse the order
        std::reverse(_v.begin(), _v.end());
    }
    
    /**
     *  Size of the array
     *  @return size_t
     */
    size_t size() const
    {
        return _v.size();
    }
    
    /**
     *  Get reference to struct
     *  @param  index
     *  @return struct addrinfo*
     */
    const struct addrinfo *operator[](int index) const
    {
        // expose vector
        return _v[index];
    }
};

/**
 *  End of namespace
 */
}
