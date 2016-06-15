/**
 *  Address.h
 *
 *  An AMQP address in the "amqp://user:password@hostname:port/vhost" notation
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class Address
{
private:
    /**
     *  Login data (username + password)
     *  @var Login
     */
    Login _login;
    
    /**
     *  The hostname
     *  @var std::string
     */
    std::string _hostname;
    
    /**
     *  Port number
     *  @var uint16_t
     */
    uint16_t _port = 5672;
    
    /**
     *  The vhost
     *  @var std::string
     */
    std::string _vhost;

public:
    /**
     *  Constructor to parse an address string
     *  The address should be in "amqp://
     *  @param  address
     *  @throws std::runtime_error
     */
    Address(const char *address) : _vhost("/")
    {
        // must start with amqp://
        if (strncmp(address, "amqp://", 7) != 0) throw std::runtime_error("AMQP address should start with \"amqp://\"");
        
        // begin of the string being parsed
        const char *begin = address + 7;
        
        // do we have a '@' to split user-data and hostname?
        const char *at = strchr(begin, '@');
        
        // do we have one?
        if (at != nullptr)
        {
            // colon could split username and password
            const char *colon = (const char *)memchr(begin, ':', at - begin);
            
            // assign the login
            _login = Login(
                std::string(begin, colon ? colon - begin : at - begin),
                std::string(colon ? colon + 1 : "", colon ? at - colon - 1 : 0)
            );
            
            // set begin to the start of the hostname
            begin = at + 1;
        }
        
        // find out where the vhost is set (starts with a slash)
        const char *slash = strchr(begin, '/');
        
        // was a vhost set?
        if (slash != nullptr && slash[1]) _vhost = slash + 1;
        
        // the hostname is everything until the slash, check is portnumber was set
        const char *colon = strchr(begin, ':');
        
        // was a portnumber specified (colon must appear before the slash of the vhost)
        if (colon && (!slash || colon < slash))
        {
            // a portnumber was set to
            _hostname.assign(begin, colon - begin);
            _port = atoi(colon + 1);
        }
        else
        {
            // no portnumber was set
            _hostname.assign(begin, slash ? slash - begin : strlen(begin));
        }
    }
    
    /**
     *  Constructor based on std::string
     *  @param  address
     */
    Address(const std::string &address) : Address(address.data()) {}

    /**
     *  Constructor based on already known properties
     *  @param  host
     *  @param  port
     *  @param  login
     *  @param  vhost
     */
    Address(std::string host, uint16_t port, Login login, std::string vhost) : _login(std::move(login)),
                                                                               _hostname(std::move(host)),
                                                                               _port(port), _vhost(std::move(vhost)) {}
    
    /**
     *  Destructor
     */
    virtual ~Address() {}
    
    /**
     *  Expose the login data
     *  @return Login
     */
    const Login &login() const
    {
        return _login;
    }
    
    /**
     *  Host name
     *  @return std::string
     */
    const std::string &hostname() const
    {
        return _hostname;
    }
    
    /**
     *  Port number
     *  @return uint16_t
     */
    uint16_t port() const
    {
        return _port;
    }
    
    /**
     *  The vhost to connect to
     *  @return std::string
     */
    const std::string &vhost() const
    {
        return _vhost;
    }
    
    /**
     *  Cast to a string
     *  @return std:string
     */
    operator std::string () const
    {
        // result object
        std::string str("amqp://");
        
        // append login
        str.append(_login.user()).append(":").append(_login.password()).append("@").append(_hostname);
        
        // do we need a special portnumber?
        if (_port != 5672) str.append(":").append(std::to_string(_port));
        
        // append default vhost
        str.append("/");
        
        // do we have a special vhost?
        if (_vhost != "/") str.append(_vhost);
        
        // done
        return str;
    }
};

/**
 *  End of namespace
 */
}

