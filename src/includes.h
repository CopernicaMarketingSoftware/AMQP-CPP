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
#include "../include/amqpcpp/classes.h"

// utility classes
#include "../include/amqpcpp/endian.h"
#include "../include/amqpcpp/buffer.h"
#include "../include/amqpcpp/bytebuffer.h"
#include "../include/amqpcpp/receivedframe.h"
#include "../include/amqpcpp/outbuffer.h"
#include "../include/amqpcpp/copiedbuffer.h"
#include "../include/amqpcpp/watchable.h"
#include "../include/amqpcpp/monitor.h"
#include "../include/amqpcpp/tcpdefines.h"

// amqp types
#include "../include/amqpcpp/field.h"
#include "../include/amqpcpp/numericfield.h"
#include "../include/amqpcpp/decimalfield.h"
#include "../include/amqpcpp/stringfield.h"
#include "../include/amqpcpp/booleanset.h"
#include "../include/amqpcpp/fieldproxy.h"
#include "../include/amqpcpp/table.h"
#include "../include/amqpcpp/array.h"

// envelope for publishing and consuming
#include "../include/amqpcpp/metadata.h"
#include "../include/amqpcpp/envelope.h"
#include "../include/amqpcpp/message.h"

// mid level includes
#include "../include/amqpcpp/exchangetype.h"
#include "../include/amqpcpp/flags.h"
#include "../include/amqpcpp/callbacks.h"
#include "../include/amqpcpp/deferred.h"
#include "../include/amqpcpp/deferredconsumer.h"
#include "../include/amqpcpp/deferredqueue.h"
#include "../include/amqpcpp/deferreddelete.h"
#include "../include/amqpcpp/deferredcancel.h"
#include "../include/amqpcpp/deferredget.h"
#include "../include/amqpcpp/channelimpl.h"
#include "../include/amqpcpp/channel.h"
#include "../include/amqpcpp/login.h"
#include "../include/amqpcpp/address.h"
#include "../include/amqpcpp/connectionhandler.h"
#include "../include/amqpcpp/connectionimpl.h"
#include "../include/amqpcpp/connection.h"
#include "../include/amqpcpp/tcphandler.h"
#include "../include/amqpcpp/tcpconnection.h"

// classes that are very commonly used
#include "../include/amqpcpp/exception.h"
#include "../include/amqpcpp/protocolexception.h"
#include "../include/amqpcpp/frame.h"
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


