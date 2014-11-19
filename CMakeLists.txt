cmake_minimum_required(VERSION 2.8)

project(amqp-cpp)

# ensure c++11 on all compilers
#include(set_cxx_norm.cmake)
#set_cxx_norm (${CXX_NORM_CXX11})

macro (add_sources)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND SRCS "${_relPath}/${_src}")
        else()
            list (APPEND SRCS "${_src}")
        endif()
    endforeach()
    if (_relPath)
        # propagate SRCS to parent directory
        set (SRCS ${SRCS} PARENT_SCOPE)
    endif()
endmacro()

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(amqp_boost_test)

include_directories(${CMAKE_SOURCE_DIR}/include)
add_library(amqp-cpp STATIC ${SRCS})

target_include_directories(amqp-cpp PRIVATE
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
$<INSTALL_INTERFACE:include>)

target_include_directories(amqp-cpp SYSTEM PUBLIC
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
$<INSTALL_INTERFACE:include>)

install(TARGETS amqp-cpp
    EXPORT amqp-cppExport
    DESTINATION lib
    INCLUDES DESTINATION include
)

file(GLOB includes_1 RELATIVE include "*.h")
install(FILES ${includes_1} DESTINATION "include/amqpcpp")
install(FILES "amqpcpp.h" DESTINATION "include")

INSTALL(EXPORT amqp-cppExport DESTINATION lib)



set(AMQP-CPP_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(AMQP-CPP_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
