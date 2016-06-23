/**
 *  Callbacks.h
 *
 *  Class storing deferred callbacks of different type.
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
#include <functional>

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Forward declarations
 */
class Message;
class MetaData;

/**
 *  All the callbacks that are supported
 *
 *  When someone registers a callback function for certain events, it should
 *  match one of the following signatures.
 */
using SuccessCallback   =   std::function<void()>;
using ErrorCallback     =   std::function<void(const char *message)>;
using FinalizeCallback  =   std::function<void()>;
using EmptyCallback     =   std::function<void()>;
using BeginCallback     =   std::function<void()>;
using HeaderCallback    =   std::function<void(const MetaData &metaData)>;
using DataCallback      =   std::function<void(const char *data, size_t size)>;
using MessageCallback   =   std::function<void(Message &&message, uint64_t deliveryTag, bool redelivered)>;
using CompleteCallback  =   std::function<void(uint64_t deliveryTag, bool redelivered)>;
using QueueCallback     =   std::function<void(const std::string &name, uint32_t messagecount, uint32_t consumercount)>;
using DeleteCallback    =   std::function<void(uint32_t deletedmessages)>;
using SizeCallback      =   std::function<void(uint32_t messagecount)>;
using ConsumeCallback   =   std::function<void(const std::string &consumer)>;
using CancelCallback    =   std::function<void(const std::string &consumer)>;

/**
 *  End namespace
 */
}
