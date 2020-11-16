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
#include "address2str.h"

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
     *  @param order
     */
    void reorder(ConnectionOrder::Order order)
    {    
        // witch on order
        switch (order)
        {
            // Do we want to have a random order of the addresses?
            // This may be useful since getaddrinfo is sorting the addresses on proximity
            // (e.g. https://lists.debian.org/debian-glibc/2007/09/msg00347.html),
            // which may break loadbalancing..
            case ConnectionOrder::Order::random:
            {
                // create a random device for the seed of the random number generator
                std::random_device rd;

                // Create the generator
                std::mt19937 gen(rd());

                // shuffle the vector.
                std::shuffle(_v.begin(), _v.end(), gen);

                // done
                break;
            }
            // do we want to sort in ascending order
            case ConnectionOrder::Order::ascending:
            {
                std::sort(_v.begin(), _v.end(), []
                (struct addrinfo * v1, struct addrinfo * v2) -> bool 
                {
                    // get the addresses
                    Address2str addr1(v1);
                    Address2str addr2(v2);
                    
                    // if addr1 doesn't have a proper address it should go to the
                    // back. Same holds for addr2
                    if (addr1.toChar() == nullptr) return false;
                    if (addr2.toChar() == nullptr) return true;

                    // make the comparison based on string comparison
                    return strcmp(addr1.toChar(), addr2.toChar()) < 0;
                });

                // done
                break;
            }

            // do we want to sort in descending order
            case ConnectionOrder::Order::descending:
            {
                std::sort(_v.begin(), _v.end(), []
                (struct addrinfo * v1, struct addrinfo * v2) -> bool
                {
                    // get the addresses
                    Address2str addr1(v1);
                    Address2str addr2(v2);
                    
                    // if addr1 doesn't have a proper address it should go to the
                    // back. Same holds for addr2
                    if (addr1.toChar() == nullptr) return false;
                    if (addr2.toChar() == nullptr) return true;

                    // make the comparison based on string comparison
                    return strcmp(addr1.toChar(), addr2.toChar()) > 0;
                });

                // done
                break;
            }

            // de we want to have reverse ordering of proximity
            case ConnectionOrder::Order::reverse:
            { 
                std::reverse(_v.begin(), _v.end());

                // done
                break;
            }

            default:
                // nothing to do, just default behaviour
                break;
        }
    }


public:
    /**
     *  Constructor
     *  @param  hostname
     *  @param  port
     *  @param  order
     */
    AddressInfo(const char *hostname, uint16_t port = 5672, ConnectionOrder::Order order = ConnectionOrder::Order::standard)
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
