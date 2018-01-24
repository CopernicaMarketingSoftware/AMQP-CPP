cmake_minimum_required(VERSION 3.1)

# project name
project(amqpcpp)

# build options
option(LINUX_TCP "Build linux sockets implementation" OFF)

# set output directory
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

# ensure c++11 on all compilers
set (CMAKE_CXX_STANDARD 11)

# set include/ as include directory
include_directories(${CMAKE_SOURCE_DIR}/include)

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

# add source files
add_subdirectory(src)
if(LINUX_TCP)
    add_subdirectory(src/linux_tcp)
endif()

# we have to prevent windows from defining the max macro. TODO more
if (WIN32)
    add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN)
endif()


# TODO Cleanup into this part of the cmakefile
option(BUILD_SHARED "build shared library" OFF)
if(BUILD_SHARED)
    add_library(${PROJECT_NAME} SHARED ${SRCS})
    set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION 2.7) # TODO version incorrect
    install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION lib
    )
else()
    add_library(${PROJECT_NAME} STATIC ${SRCS})
    install(TARGETS ${PROJECT_NAME}
            ARCHIVE DESTINATION lib
    )
endif()
Include_directories(${PROJECT_SOURCE_DIR})
install(DIRECTORY include/ DESTINATION include/amqpcpp
        FILES_MATCHING PATTERN "*.h")
install(FILES amqpcpp.h DESTINATION include)

set(AMQP-CPP_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(AMQP-CPP_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)