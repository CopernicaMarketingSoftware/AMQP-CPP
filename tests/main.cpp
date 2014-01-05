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
#include <libamqp.h>
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
    // create connection
    MyConnection connection;

    // start the main event loop
    Event::MainLoop::instance()->run();

    // done
    return 0;
}

