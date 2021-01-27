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
#include "authentication.h"
#include <string>

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class Login : public Authentication
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
    virtual ~Login() = default;

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
     *
     *  @return string
     */
    std::string mechanism() const override
    {
        return "PLAIN";
    }

    /**
     *  String representation in SASL PLAIN mode
     *  @return string
     */
    std::string response() const override
    {
        // we need an initial string
        std::string result("\0", 1);

        // append other elements
        return result.append(_user).append("\0",1).append(_password);
    }

    /**
     *  Create string from login
     *  @return std::string
     */
    virtual std::string toString() const
    {
        return _user + ":" + _password;
    }

private:
    /**
     *  Is the login set?
     *  @return bool
     */
    bool isSet() const override
    {
        return !_user.empty() || !_password.empty();
    }

    /**
     *  Compare authentication
     *  @return bool (negative if this < that, zero if this == that, positve if this > that)
     */
    int compare(const Authentication& that) const override
    {
        // At this point the Authentication class made sure that `that` has the
        // same mechanism as `this`
        const Login& that_login = dynamic_cast<const Login&>(that);

        if (_user < that_login._user)
        {
            return -1;
        }
        else if (_user > that_login._user)
        {
            return 1;
        }

        if (_password < that_login._password)
        {
            return -1;
        }
        else if (_password > that_login._password)
        {
            return 1;
        }

        return 0;
    }

    /**
     *  Function to allow writing the login to a stream
     *  @param  stream
     *  @return std::ostream
     */
    std::ostream &print(std::ostream &stream)
    {
        // write username and password
        return stream << _user << ":" << _password;
    }
};

/**
 *  End of namespace
 */
}

