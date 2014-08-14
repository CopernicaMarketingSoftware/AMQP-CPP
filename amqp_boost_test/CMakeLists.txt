set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.55 REQUIRED COMPONENTS thread system)

if(NOT Boost_FOUND)
	message( FATAL_ERROR "boost must be installed")
endif()

include_directories(${Boost_INCLUDE_DIRS})

set(SRC main.cpp)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
set(SPECIFIC_HOST_LIBS rt pthread)
else()
set(SPECIFIC_HOST_LIBS)
endif()

set(LIBS
${Boost_LIBRARIES}
amqp-cpp
)

include_directories(SYSTEM ${CRYPTLIB_INCLUDE_PATH})
link_directories(${Boost_LIBRARY_PATH})

include_directories(SYSTEM ${AMQP-CPP_INCLUDE_PATH})

add_executable(amqp_boost_test ${SRC})
target_link_libraries(amqp_boost_test ${LIBS} ${SPECIFIC_HOST_LIBS})

install(TARGETS amqp_boost_test
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
