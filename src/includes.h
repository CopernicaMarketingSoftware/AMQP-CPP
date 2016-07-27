/**
 *  Includes.h
 *
 *  The includes that are necessary to compile the AMQP library
 *  This file also holds includes that may not be necessary for including the library
 *
 *  @documentation private
 */

// c and c++ dependencies
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string>
#include <memory>
#include <limits>
#include <ostream>
#include <math.h>
#include <map>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <queue>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <functional>
#include <stdexcept>

// forward declarations
#include "../include/classes.h"

// utility classes
#include "../include/endian.h"
#include "../include/buffer.h"
#include "../include/bytebuffer.h"
#include "../include/receivedframe.h"
#include "../include/outbuffer.h"
#include "../include/watchable.h"
#include "../include/monitor.h"
#include "../include/tcpdefines.h"

// amqp types
#include "../include/field.h"
#include "../include/numericfield.h"
#include "../include/decimalfield.h"
#include "../include/stringfield.h"
#include "../include/booleanset.h"
#include "../include/fieldproxy.h"
#include "../include/table.h"
#include "../include/array.h"

// envelope for publishing and consuming
#include "../include/metadata.h"
#include "../include/envelope.h"
#include "../include/message.h"

// mid level includes
#include "../include/exchangetype.h"
#include "../include/flags.h"
#include "../include/callbacks.h"
#include "../include/deferred.h"
#include "../include/deferredconsumer.h"
#include "../include/deferredqueue.h"
#include "../include/deferreddelete.h"
#include "../include/deferredcancel.h"
#include "../include/deferredget.h"
#include "../include/channelimpl.h"
#include "../include/channel.h"
#include "../include/login.h"
#include "../include/address.h"
#include "../include/connectionhandler.h"
#include "../include/connectionimpl.h"
#include "../include/connection.h"
#include "../include/tcphandler.h"
#include "../include/tcpconnection.h"

// classes that are very commonly used
#include "exception.h"
#include "protocolexception.h"
#include "frame.h"
#include "extframe.h"
#include "methodframe.h"
#include "headerframe.h"
#include "connectionframe.h"
#include "channelframe.h"
#include "exchangeframe.h"
#include "queueframe.h"
#include "basicframe.h"
#include "transactionframe.h"
#include "addressinfo.h"


