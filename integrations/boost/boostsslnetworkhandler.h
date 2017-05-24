/**
 *  Classes and includes for ssl support for boost asio network operations
 * 
 *  @copyright 2014 TeamSpeak Systems GmbH
 */
 
#ifndef AMQP_CPP_BOOST_SSL_NETWORK_HANDLER_H
#define AMQP_CPP_BOOST_SSL_NETWORK_HANDLER_H

#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/rfc2818_verification.hpp>

/**
 *  Set up namespace
 */
namespace AMQP {
template <>
class PostConnectHandler<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>{
public:
  template <typename CB>
  void afterConnect(boost::asio::ssl::stream<boost::asio::ip::tcp::socket>& socket, CB cb)
  {
	  socket.async_handshake( boost::asio::ssl::stream<boost::asio::ip::tcp::socket>::client, cb);
  }
};

/**
* end namespace
*/
}

#endif //AMQP_CPP_BOOST_SSL_NETWORK_HANDLER_H