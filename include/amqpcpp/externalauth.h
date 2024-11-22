/**
 *  Authentication for EXTERNAL
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
#include "authentication.h"

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class ExternalAuth : public Authentication
{
public:
    /**
     *
     *  @return string
     */
    std::string mechanism() const override
    {
        return "EXTERNAL";
    }

};

/**
 *  End of namespace
 */
}

