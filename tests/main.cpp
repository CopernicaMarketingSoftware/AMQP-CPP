/**
 *  Main.cpp
 *
 *  Test program
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Global libraries that we need
 */
#include <amqpcpp.h>
#include <copernica/network.h>

/**
 *  Namespaces to use
 */
using namespace std;
using namespace Copernica;

/**
 *  Local libraries
 */
#include "myconnection.h"

/**
 *  Main procedure
 *  @param  argc
 *  @param  argv
 *  @return int
 */
int main(int argc, const char *argv[])
{   
    // need an ip
    if (argc != 2)
    {
        // report error
        std::cerr << "usage: " << argv[0] << " <ip>" << std::endl;
        
        // done
        return -1;
    }
    else
    {
        // create connection
        MyConnection connection(argv[1]);

        // start the main event loop
        Event::MainLoop::instance()->run();

        // done
        return 0;
    }
}