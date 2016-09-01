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
     *  The login mechanism
     *  @var LoginMechanism
     */
    LoginMechanism _mechanism;

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
     *  Login mechanism enum
     */
    typedef enum
    {
        LOGIN_PLAIN,
        LOGIN_EXTERNAL
    } LoginMechanism;

    /**
     *  Default constructor
     */
    Login() : _mechanism(LOGIN_PLAIN), _user("guest"), _password("guest") {}

    /**
     *  Constructor
     *  @param  user
     *  @param  password
     */
    Login(std::string user, std::string password) :
        Login(), _user(std::move(user)), _password(std::move(password)) {}

    /**
     *  Constructor for EXTERNAL mechanism
     */
    Login(LoginMechanism mechanism) : Login(), _mechanism(mechanism) {}

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
     *  Retrieve login mechanism string representation
     *  @return LoginMechanism
     */
    std::string mechanismRepr() const
    {
        switch (_mechanism)
        {
            case LOGIN_PLAIN:
                return "PLAIN";
            case LOGIN_EXTERNAL:
                return "EXTERNAL";
            default:
                return "";
        }
    }

    /**
     *  String representation in SASL PLAIN mode
     *  @return string
     */
    std::string stringRepr() const
    {
        // we need an initial string
        std::string result("\0", 1);

        if (_mechanism == LOGIN_PLAIN) {
            // append login and password info for plain login
            return result.append(_user).append("\0",1).append(_password);
        }

        return result;
    }
};

/**
 *  End of namespace
 */
}

