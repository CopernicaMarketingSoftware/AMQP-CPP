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

option(BUILD_SHARED "build shared library" OFF)

if(BUILD_SHARED)
  add_library(amqp-cpp SHARED ${SRCS})
  set_target_properties(amqp-cpp PROPERTIES SOVERSION 2.6)
  install(TARGETS amqp-cpp
          LIBRARY DESTINATION lib
  )
else()
  add_library(amqp-cpp STATIC ${SRCS})
  install(TARGETS amqp-cpp
          ARCHIVE DESTINATION lib
  )
endif()
Include_directories(${PROJECT_SOURCE_DIR})
install(DIRECTORY include/ DESTINATION include/amqpcpp
        FILES_MATCHING PATTERN "*.h")
install(FILES amqpcpp.h DESTINATION include)

option(BUILD_TUTORIALS "build rabbitmq tutorials" OFF)
if(BUILD_TUTORIALS)
    add_subdirectory(examples/rabbitmq_tutorials)
endif()

set(AMQP-CPP_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(AMQP-CPP_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

if (WIN32)
    add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN)
endif()
