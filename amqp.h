/**
 *  AMQP.h
 *
 *  Starting point for all includes of the Copernica AMQP library
 *
 *  @documentation public
 */

// base C++ include files
#include <vector>
#include <cstring>
#include <sstream>
#include <string>
#include <limits>
#include <memory>
#include <type_traits>
#include <cstdlib>
#include <map>


// base C include files
#include <stdint.h>
#include <math.h>
#include <stddef.h>
#include <endian.h>
#include <string.h>

// other libraries
#include <copernica/event.h>
#include <copernica/network.h>

// forward declarations
#include <copernica/amqp/classes.h>

// utility classes
#include <copernica/amqp/receivedframe.h>
#include <copernica/amqp/outbuffer.h>
#include <copernica/amqp/watchable.h>
#include <copernica/amqp/monitor.h>

// amqp types
#include <copernica/amqp/field.h>
#include <copernica/amqp/numericfield.h>
#include <copernica/amqp/decimalfield.h>
#include <copernica/amqp/stringfield.h>
#include <copernica/amqp/booleanset.h>
#include <copernica/amqp/fieldproxy.h>
#include <copernica/amqp/table.h>
#include <copernica/amqp/array.h>

// envelope for publishing and consuming
#include <copernica/amqp/envelopefield.h>
#include <copernica/amqp/envelope.h>

// mid level includes
#include <copernica/amqp/exchangetype.h>
#include <copernica/amqp/flags.h>
#include <copernica/amqp/channelhandler.h>
#include <copernica/amqp/channelimpl.h>
#include <copernica/amqp/channel.h>
#include <copernica/amqp/login.h>
#include <copernica/amqp/connectionhandler.h>
#include <copernica/amqp/connectionimpl.h>
#include <copernica/amqp/connection.h>




