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
     *  The vhost
     *  @var string
     */
    std::string _vhost;
    
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
     *  @param  vhost
     *  @param  user
     *  @param  password
     */
    Login(const std::string &vhost, const std::string &user, const std::string &password) :
        _vhost(vhost), _user(user), _password(password) {}

    /**
     *  Constructor
     *  @param  user
     *  @param  password
     */
    Login(const std::string &user, const std::string &password) :
        _vhost("/"), _user(user), _password(password) {}
        
    /**
     *  Constructor
     */
    Login() :
        _vhost("/"), _user("guest"), _password("guest") {}
        
    /**
     *  Destructor
     */
    virtual ~Login() {}
    
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
    
    /**
     *  Retrieve the vhost
     *  @return string
     */
    std::string &vhost()
    {
        return _vhost;
    }
};

/**
 *  End of namespace
 */
}

