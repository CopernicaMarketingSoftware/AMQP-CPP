/**
 *  The authentication information to access a server
 *
 *  This class may provide a SASL PLAIN or EXTERNAL authentication
 *
 *  @copyright 2014 - 2021 Copernica BV
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
class Authentication
{
public:
    /**
     *  Destructor
     */
    virtual ~Authentication() = default;
    
    /**
     *  Cast to boolean: is the login set?
     *  @return bool
     */
    operator bool () const
    {
        return isSet();
    }
    
    /**
     *  Negate operator: is it not set
     *  @return bool
     */
    bool operator! () const
    {
        return !isSet();
    }

    /**
     *  
     *  @return string
     */
    virtual std::string mechanism() const = 0;

    /**
     *  
     *  @return string
     */
    virtual std::string response() const
    {
        return "";
    }
    
    /**
     *  Comparison operator
     *  @param  that
     *  @return bool
     */
    bool operator==(const Authentication &that) const
    {
        if (mechanism() == that.mechanism())
            return compare(that) == 0;
        else
            return false;
    }

    /**
     *  Comparison operator
     *  @param  that
     *  @return bool
     */
    bool operator!=(const Authentication &that) const
    {
        // the opposite of operator==
        return !operator==(that);
    }
    
    /**
     *  Comparison operator
     *  @param  that
     *  @return bool
     */
    bool operator<(const Authentication &that) const
    {
        if (mechanism() == that.mechanism())
            return compare(that) < 0;
        else
            return mechanism() < that.mechanism();
    }

    /**
     *  Create string from authentication
     *  @return std::string
     */
    virtual std::string toString() const
    {
        return "";
    }

    /**
     *  Friend function to allow writing the login to a stream
     *  @param  stream
     *  @param  login
     *  @return std::ostream
     */
    friend std::ostream &operator<<(std::ostream &stream, const Authentication &auth)
    {
        return auth.print(stream);
    }

private:
    /**
     *  Is the authentication set?
     *  @return bool
     */
    virtual bool isSet() const = 0;

    /**
     *  Compare authentication
     *  @return bool (negative if this < that, zero if this == that, positve if this > that)
     */
    virtual int compare(const Authentication& that) const = 0;

    /**
     *  Write the authentication to a stream
     *  @param  stream
     *  @return std::ostream
     */
    virtual std::ostream &print(std::ostream &stream) const
    {
        return stream;
    }
};

/**
 *  End of namespace
 */
}

