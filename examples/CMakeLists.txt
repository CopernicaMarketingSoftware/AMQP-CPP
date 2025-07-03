

###################################
# Boost
###################################

add_executable(amqpcpp_boost_example libboostasio.cpp)

add_dependencies(amqpcpp_boost_example amqpcpp)

target_link_libraries(amqpcpp_boost_example amqpcpp boost_system pthread dl)


add_executable(amqpcpp_boost_ssl_example libboostasio_ssl.cpp)

add_dependencies(amqpcpp_boost_ssl_example amqpcpp)

target_link_libraries(amqpcpp_boost_ssl_example amqpcpp boost_system pthread dl ssl crypto)


add_executable(amqpcpp_boost_multithreading_example libboostasio_multithreading.cpp)

add_dependencies(amqpcpp_boost_multithreading_example amqpcpp)

target_link_libraries(amqpcpp_boost_multithreading_example amqpcpp boost_system pthread dl)


if(AMQP-CPP_LINUX_TCP)
###################################
# Libev
###################################

add_executable(amqpcpp_libev_example libev.cpp)

add_dependencies(amqpcpp_libev_example amqpcpp)

target_link_libraries(amqpcpp_libev_example amqpcpp ev pthread dl ssl)


###################################
# Libuv
###################################

add_executable(amqpcpp_libuv_example libuv.cpp)

add_dependencies(amqpcpp_libuv_example amqpcpp)

target_link_libraries(amqpcpp_libuv_example amqpcpp uv pthread dl ssl)
endif(AMQP-CPP_LINUX_TCP)
