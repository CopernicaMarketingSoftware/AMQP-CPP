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
};

/**
 *  End of namespace
 */
}

