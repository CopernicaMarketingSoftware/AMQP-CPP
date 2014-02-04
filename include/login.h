/**
 *  The login information to access a server
 *
 *  This class combines login, password and vhost
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
     *  Constructor
     *  @param  user
     *  @param  password
     */
    Login(const std::string &user, const std::string &password) :
        _user(user), _password(password) {}

    /**
     *  Copy constructor
     *  @param  login
     */
    Login(const Login &login) :
        _user(login._user), _password(login._password) {}

    /**
     *  Constructor
     */
    Login() :
        _user("guest"), _password("guest") {}
        
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
    std::string saslPlain()
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

