/**
 *  The login information to access a server
 *
 *  This class combines login, password and vhost
 *
 *  @copyright 2014 - 2018 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <string>

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class Login
{
private:
    /**
     *  The username
     *  @var string
     */
    std::string _user;

    /**
     *  The password
     *  @var string
     */
    std::string _password;

    /**
     *  The security mechanism
     *  @var string
     */
    std::string _mechanism;

    /**
     *  The security response
     *  @var string
     */
    std::string _response;


public:
    /**
     *  Default constructor with PLAIN authentication
     */
    Login() : _user("guest"), _password("guest")
    {
        // Set default PLAIN mechanism and response
        _mechanism = "PLAIN";
        _response = saslPlain();
    }

    /**
     *  Constructor with PLAIN authentication mechanism
     *
     *  @param  user
     *  @param  password
     */
    Login(std::string user, std::string password) :
        _user(std::move(user)), _password(std::move(password))
    {
        // Set default PLAIN mechanism and response
        _mechanism = "PLAIN";
        _response = saslPlain();
    }

    /**
     *  Constructor
     *  @param  user
     *  @param  password
     */
    Login(const char *user, const char *password) :
        _user(user), _password(password)
    {
        // Set default PLAIN mechanism and response
        _mechanism = "PLAIN";
        _response = saslPlain();
    }

    /**
     *  Constructor with custom authentication mechanism
     *  @param  user
     *  @param  password
     *  @param  mechanism
     *  @param  response
     */
    Login(std::string user, std::string password, std::string mechanism, std::string response)
        : _user(std::move(user))
        , _password(std::move(password))
        , _mechanism(std::move(mechanism))
        , _response(std::move(response)) {}

    /**
     *  Constructor with custom authentication mechanism
     *  @param  user
     *  @param  password
     *  @param  mechanism
     *  @param  response
     */
    Login(const char *user, const char *password, const char *mechanism, const char *response)
        : _user(user), _password(password), _mechanism(mechanism), _response(response) {}

    /**
     *  Destructor
     */
    virtual ~Login() = default;
    
    /**
     *  Cast to boolean: is the login set?
     *  @return bool
     */
    operator bool () const
    {
        return !_user.empty() || !_password.empty() || !_mechanism.empty() || !_response.empty();
    }
    
    /**
     *  Negate operator: is it not set
     *  @return bool
     */
    bool operator! () const
    {
        return _user.empty() && _password.empty() && _mechanism.empty() && _response.empty();
    }

    /**
     *  Retrieve the user name
     *  @return string
     */
    const std::string &user() const
    {
        return _user;
    }

    /**
     *  Retrieve the password
     *  @return string
     */
    const std::string &password() const
    {
        return _password;
    }

    /**
     *  Retrieve the security mechanism
     *  @return string
     */
    const std::string &mechanism() const
    {
        return _mechanism;
    }

    /**
     *  String representation for security response
     *  @return string
     */
    const std::string &response() const
    {
        return _response;
    }

    /**
     *  String representation in SASL PLAIN mode
     *  @return string
     */
    std::string saslPlain() const
    {
        // we need an initial string
        std::string result("\0", 1);

        // append other elements
        return result.append(_user).append("\0",1).append(_password);
    }

    /**
     *  Comparison operator
     *  @param  that
     *  @return bool
     */
    bool operator==(const Login &that) const
    {
        // username and password must match
        return _user == that._user           &&
               _password == that._password   &&
               _mechanism == that._mechanism &&
               _response == that._response;
    }

    /**
     *  Comparison operator
     *  @param  that
     *  @return bool
     */
    bool operator!=(const Login &that) const
    {
        // the opposite of operator==
        return !operator==(that);
    }
    
    /**
     *  Comparison operator
     *  @param  that
     *  @return bool
     */
    bool operator<(const Login &that) const
    {
        // compare users
        if (_user != that._user)           return _user < that._user;
        // compare passwords
        if (_password != that._password)   return _password < that._password;
        // compare mechanisms
        if (_mechanism != that._mechanism) return _mechanism < that._mechanism;
        // compare responses
        return _response < that._response;
    }

    /**
     *  Friend function to allow writing the login to a stream
     *  @param  stream
     *  @param  login
     *  @return std::ostream
     */
    friend std::ostream &operator<<(std::ostream &stream, const Login &login)
    {
        // write username and password
        return stream << login._user      << ":"
                      << login._password  << ":"
                      << login._mechanism << ":"
                      << login._response;
    }

};

/**
 *  End of namespace
 */
}

