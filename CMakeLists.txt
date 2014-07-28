cmake_minimum_required(VERSION 2.8)

project(amqp-cpp)

# ensure c++11 on all compilers
include(set_cxx_norm.cmake)
set_cxx_norm (${CXX_NORM_CXX11})

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

include_directories(${CMAKE_SOURCE_DIR}/include)
add_library(amqp-cpp STATIC ${SRCS})
target_include_directories(amqp-cpp SYSTEM PUBLIC ${PROJECT_SOURCE_DIR})

set(AMQP-CPP_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(AMQP-CPP_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
