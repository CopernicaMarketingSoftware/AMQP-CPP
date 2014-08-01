/**
 *  Buffer.h
 *
 *  Interface that can be implemented by client applications and that
 *  is parsed to the Connection::parse() method.
 *
 *  Normally, the Connection::parse() method is fed with a byte
 *  array. However, if you're receiving big frames, it may be inconvenient
 *  to copy these big frames into continguous byte arrays, and you
 *  prefer using objects that internally use linked lists or other
 *  ways to store the bytes. In such sitations, you can implement this
 *  interface and pass that to the connection.
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2014 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class Buffer
{


};

/**
 *  End of namespace
 */
}


