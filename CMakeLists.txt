# Builds AMQP-CPP
#
# Options:
#
# - AMQP-CPP_BUILD_SHARED (default OFF)
#       ON:  Build shared lib
#       OFF: Build static lib
#
# - AMQP-CPP_LINUX_TCP (default OFF)
#       ON:  Build posix handler implementation
#       OFF: Don't build posix handler implementation

cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

# project name
project(amqpcpp)

# build options
option(AMQP-CPP_BUILD_SHARED "Build shared library. If off, build will be static." OFF)
option(AMQP-CPP_LINUX_TCP "Build linux sockets implementation." OFF)
option(AMQP-CPP_BUILD_EXAMPLES "Build amqpcpp examples" OFF)

# ensure c++11 on all compilers
set (CMAKE_CXX_STANDARD 11)

# add source files
# ------------------------------------------------------------------------------------------------------

# set include/ as include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# macro that adds a list of provided source files to a list called SRCS.
# if variable SRCS does not yet exist, it is created.
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
if(AMQP-CPP_LINUX_TCP)
    add_subdirectory(src/linux_tcp)
endif()

# potentially build the examples
if(AMQP-CPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# settings for specific compilers
# ------------------------------------------------------------------------------------------------------

# we have to prevent windows from defining the max macro.
if (WIN32)
    add_definitions(-DNOMINMAX)
endif()

# build targets
# ------------------------------------------------------------------------------------------------------

# set output directory
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)

if(AMQP-CPP_BUILD_SHARED)
    # create shared lib
    add_library(${PROJECT_NAME} SHARED ${SRCS})
    # set shared lib version
    set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION 3.0)
else()
    # create static lib
    add_library(${PROJECT_NAME} STATIC ${SRCS})
endif()

# install rules
# ------------------------------------------------------------------------------------------------------

if(AMQP-CPP_BUILD_SHARED)
    # copy shared lib and its static counter part
    install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Config
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION lib
            RUNTIME DESTINATION lib
    )
else()
    # copy static lib
    install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Config
            ARCHIVE DESTINATION lib
    )
endif()

# copy header files
install(DIRECTORY include/amqpcpp/ DESTINATION include/amqpcpp
        FILES_MATCHING PATTERN "*.h")
install(FILES include/amqpcpp.h DESTINATION include)

install(EXPORT ${PROJECT_NAME}Config DESTINATION cmake)
export(TARGETS ${PROJECT_NAME} FILE ${PROJECT_NAME}Config.cmake)
