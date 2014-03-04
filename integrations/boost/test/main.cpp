/**
 *  Main.cpp
 *
 *  Test program
 *
 *  @copyright 2014 Copernica BV
 *  @copyright 2014 TeamSpeak Systems GmbH
 */

//uncomment the line below to test ssl connections.
//to test, make sure ca.pem, cert.pem and key.pem are in the current working dir
//#define SSL_DEMO

/**
 *  Global libraries that we need
 */
#include <amqpcpp.h>
#include <boost/asio.hpp>

/**
 *  Namespaces to use
 */
using namespace std;

/**
 *  Local libraries
 */
#include "myconnection.h"

#ifdef SSL_DEMO
  static const std::uint16_t SERVER_PORT=5671;
  #include "../boostsslnetworkhandler.h"
#else
  static const std::uint16_t SERVER_PORT=5672;
#endif

/**
 *  Main procedure
 *  @param  argc
 *  @param  argv
 *  @return int
 */
 
int main(int argc, const char *argv[])
{   
    // need an ip/host name
    if (argc != 2)
    {
        // report error
        std::cerr << "usage: " << argv[0] << " <ip/host>" << std::endl;
        
        // done
        return -1;
    }
    else
    {
		//create an io service
		boost::asio::io_service io_service;

#ifdef SSL_DEMO
		//create an ssl context
		boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv1_client);
		
		//set verify mode
		ctx.set_verify_mode(boost::asio::ssl::verify_peer |  boost::asio::ssl::verify_fail_if_no_peer_cert );

		//load the ca certificate so we can check the server
		ctx.load_verify_file("ca.pem");

		//set the callback for checking the server cert
		ctx.set_verify_callback(boost::asio::ssl::rfc2818_verification(argv[1]));
		
		//now load client cert+key so we can auth ourselves to the server
		ctx.use_certificate_file("cert.pem", boost::asio::ssl::context::pem);
		ctx.use_private_key_file("key.pem", boost::asio::ssl::context::pem);
		
		//create the socket
		boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket(io_service, ctx);		
#else
		//create the socket
		boost::asio::ip::tcp::socket socket(io_service);
#endif		
		//create connection object
		AMQP::BoostNetworkHandler<decltype(socket)> networkHandler(socket);
		
        // create connection
        MyConnection connection(&networkHandler, argv[1], SERVER_PORT);

        // start the main event loop
        io_service.run();

        // done
        return 0;
    }
}
