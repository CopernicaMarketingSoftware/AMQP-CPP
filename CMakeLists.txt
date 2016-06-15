cmake_minimum_required(VERSION 2.8)

project(amqp-cpp)

set(amqp-cpp_MAJOR "2")
set(amqp-cpp_MINOR "5")
set(amqp-cpp_MICRO "2")

set(CPACK_PACKAGE_VERSION_MAJOR ${amqp-cpp_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${amqp-cpp_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${amqp-cpp_MICRO})

set(CPP_VERSION "c++11" CACHE STRING "C++ version to use")
set_property(CACHE CPP_VERSION PROPERTY STRINGS "c++11" "c++14" "c++1y")

if (${CMAKE_CXX_COMPILER_ID} MATCHES GNU OR ${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=${CPP_VERSION}" HAVE_CPP_VERSION)
    add_definitions("-std=${CPP_VERSION}")
    add_definitions("-Wextra -Wall -Wno-unused-parameter")
endif()

if (WIN32)
    add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN)
endif()

include_directories(include/)

add_subdirectory(include)
add_subdirectory(src)

option(BUILD_TUTORIALS "build rabbitmq tutorials" OFF)
if(BUILD_TUTORIALS)
    add_subdirectory(examples/)
endif()

set(CPACK_GENERATOR "TGZ")
include(CPack)
