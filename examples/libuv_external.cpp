/**
 *  libuv_external.cpp
 * 
 *  Test program to check AMQP functionality based on LibUV
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 - 2021 Copernica BV
 */

/**
 *  Dependencies
 */
#include <openssl/ssl.h>
#include <uv.h>
#include <amqpcpp.h>
#include <amqpcpp/libuv.h>

/**
 *  Custom handler
 */
class MyHandler : public AMQP::LibUvHandler
{
private:
    /**
     *  Method that is called when a connection error occurs
     *  @param  connection
     *  @param  message
     */
    virtual void onError(AMQP::TcpConnection *connection, const char *message) override
    {
        std::cout << "error: " << message << std::endl;
    }

    /**
     *  Method that is called when the TCP connection ends up in a connected state
     *  @param  connection  The TCP connection
     */
    virtual void onConnected(AMQP::TcpConnection *connection) override 
    {
        std::cout << "connected" << std::endl;
    }

    virtual bool onSecuring(AMQP::TcpConnection *connection, SSL* ssl) override
    {
        SSL_CTX* ctx = SSL_get_SSL_CTX(ssl);
        if (!ctx)
        {
            std::cerr << "Cannot load SSL context." << std::endl;
            return false;
        }
        if (SSL_CTX_load_verify_locations(ctx, "ca_certificate.pem", nullptr) != 1)
        {
            std::cerr << "Cannot load CA certificate." << std::endl;
            return false;
        }
        if (SSL_use_certificate_file(ssl, "client_certificate.pem", SSL_FILETYPE_PEM) != 1)
        {
            std::cerr << "Cannot load client certificate." << std::endl;
            return false;
        }
        if (SSL_use_PrivateKey_file(ssl, "private_key.pem", SSL_FILETYPE_PEM) != 1)
        {
            std::cerr << "Cannot load private key." << std::endl;
            return false;
        }
        return true;
    }

    virtual bool onSecured(AMQP::TcpConnection *connection, const SSL* ssl) override
    {
        if (SSL_get_verify_result(ssl) != X509_V_OK)
          return false;

        X509* peer = SSL_get_peer_certificate(ssl);
        if (!peer)
            return false;

        X509_free(peer);

        return true;
    }
    
public:
    /**
     *  Constructor
     *  @param  uv_loop
     */
    MyHandler(uv_loop_t *loop) : AMQP::LibUvHandler(loop) {}

    /**
     *  Destructor
     */
    virtual ~MyHandler() = default;
};

/**
 *  Main program
 *  @return int
 */
int main()
{
    // access to the event loop
    auto *loop = uv_default_loop();
    
    // handler for libev
    MyHandler handler(loop);
    
    // make a connection
    AMQP::TcpConnection connection(&handler, AMQP::Address("amqp://localhost/"), std::make_shared<AMQP::ExternalAuth>());
    
    // we need a channel too
    AMQP::TcpChannel channel(&connection);
    
    // create a temporary queue
    channel.declareQueue(AMQP::exclusive).onSuccess([&connection](const std::string &name, uint32_t messagecount, uint32_t consumercount) {
        
        // report the name of the temporary queue
        std::cout << "declared queue " << name << std::endl;
    });
    
    // run the loop
    uv_run(loop, UV_RUN_DEFAULT);

    // done
    return 0;
}

