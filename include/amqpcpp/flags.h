#pragma once
/**
 *  AmqpFlags.h
 *
 *  The various flags that are supported
 *
 *  @copyright 2014 Copernica BV
 */
 
/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  All bit flags
 *  @var int
 */
extern const int durable;
extern const int autodelete;
extern const int active;
extern const int passive;
extern const int ifunused;
extern const int ifempty;
extern const int global;
extern const int nolocal;
extern const int noack;
extern const int exclusive;
extern const int mandatory;
extern const int immediate;
extern const int redelivered;
extern const int multiple;
extern const int requeue;

/**
 *  End of namespace
 */
}

