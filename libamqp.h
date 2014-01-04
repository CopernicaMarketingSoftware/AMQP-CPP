/**
 *  AMQP.h
 *
 *  Starting point for all includes of the Copernica AMQP library
 *
 *  @documentation public
 */

// base C++ include files
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <queue>
#include <set>
#include <limits>
#include <cstddef>
#include <cstring>

// base C include files
#include <stdint.h>
#include <math.h>
#include <endian.h>

// forward declarations
#include <libamqp/classes.h>

// utility classes
#include <libamqp/receivedframe.h>
#include <libamqp/outbuffer.h>
#include <libamqp/watchable.h>
#include <libamqp/monitor.h>

// amqp types
#include <libamqp/field.h>
#include <libamqp/numericfield.h>
#include <libamqp/decimalfield.h>
#include <libamqp/stringfield.h>
#include <libamqp/booleanset.h>
#include <libamqp/fieldproxy.h>
#include <libamqp/table.h>
#include <libamqp/array.h>

// envelope for publishing and consuming
#include <libamqp/envelopefield.h>
#include <libamqp/envelope.h>

// mid level includes
#include <libamqp/exchangetype.h>
#include <libamqp/flags.h>
#include <libamqp/channelhandler.h>
#include <libamqp/channelimpl.h>
#include <libamqp/channel.h>
#include <libamqp/login.h>
#include <libamqp/connectionhandler.h>
#include <libamqp/connectionimpl.h>
#include <libamqp/connection.h>

