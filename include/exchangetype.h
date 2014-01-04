/**
 *  ExchangeType.h
 *
 *  The various exchange types that are supported
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  The class
 */
enum ExchangeType
{
    fanout,
    direct,
    topic,
    headers
};

/**
 *  End of namespace
 */
}

