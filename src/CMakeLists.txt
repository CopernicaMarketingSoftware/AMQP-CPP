set(sources
    array.cpp
    channelimpl.cpp
    connectionimpl.cpp
    deferredcancel.cpp
    deferredconsumer.cpp
    deferredget.cpp
    field.cpp
    flags.cpp
    receivedframe.cpp
    table.cpp
    tcpconnection.cpp
    watchable.cpp
)

add_library(amqp-cpp ${sources})
install(TARGETS amqp-cpp
    EXPORT amqp-cpp-targets
    LIBRARY DESTINATION lib COMPONENT library
    ARCHIVE DESTINATION lib COMPONENT library)
