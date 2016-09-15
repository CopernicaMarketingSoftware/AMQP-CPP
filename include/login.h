/**
 *  The login information to access a server
 *
 *  This class combines login, password and vhost
 *
 *  @copyright 2014 Copernica BV
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


public:
    /**
     *  Default constructor
     */
    Login() : _user("guest"), _password("guest") {}

    /**
     *  Constructor
     *  @param  user
     *  @param  password
     */
    Login(std::string user, std::string password) :
        _user(std::move(user)), _password(std::move(password)) {}

    /**
     *  Constructor
     *  @param  user
     *  @param  password
     */
    Login(const char *user, const char *password) :
        _user(user), _password(password) {}

    /**
     *  Destructor
     */
    virtual ~Login() {}

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
};

/**
 *  End of namespace
 */
}

