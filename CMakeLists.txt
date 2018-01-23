cmake_minimum_required(VERSION 3.1)

project(amqpcpp)

# ensure c++11 on all compilers
set (CMAKE_CXX_STANDARD 11)

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
if(BUILD_TCP)
    add_subdirectory(src/tcpconnection)
endif()

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

if (WIN32)
    add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN)
endif()
