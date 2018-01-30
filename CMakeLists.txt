cmake_minimum_required(VERSION 2.8)

project(amqpcpp)

# ensure c++11 on all compilers
include(set_cxx_norm.cmake)
set_cxx_norm (${CXX_NORM_CXX11})


# get version from TAG  on git
include(GetGitRevisionDescription.cmake)
git_describe(GIT_TAG_VERSION --tags)

# parse the version information into pieces.
# http://ipenguin.ws/2012/11/cmake-automatically-use-git-tags-as.html
string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${GIT_TAG_VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${GIT_TAG_VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${GIT_TAG_VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+(.*)" "\\1" VERSION_SHA1 "${GIT_TAG_VERSION}")
set(VERSION_SHORT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
set(VERSION_SONAME "${VERSION_MAJOR}.${VERSION_MINOR}")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Makefile.in" "${CMAKE_CURRENT_SOURCE_DIR}/Makefile")


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
  add_library(amqpcpp SHARED ${SRCS})
  set_target_properties(amqpcpp PROPERTIES SOVERSION ${VERSION_SONAME})
  install(TARGETS amqpcpp
          LIBRARY DESTINATION lib
  )
else()
  add_library(amqpcpp STATIC ${SRCS})
  install(TARGETS amqpcpp
          ARCHIVE DESTINATION lib
  )
endif()

# if UNIX build package file
if(UNIX)
    set(DEST_DIR "${CMAKE_INSTALL_PREFIX}")

    include(FindPkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        # Produce a pkg-config file for linking against the shared lib
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/amqpcpp.pc.in" "${CMAKE_CURRENT_SOURCE_DIR}/amqpcpp.pc" @ONLY)
        install(FILES        "${CMAKE_CURRENT_SOURCE_DIR}/amqpcpp.pc"
                DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")
     endif()
endif()


Include_directories(${PROJECT_SOURCE_DIR})
install(DIRECTORY include/ DESTINATION include/amqpcpp
        FILES_MATCHING PATTERN "*.h")
install(FILES amqpcpp.h DESTINATION include)

option(BUILD_TUTORIALS "build rabbitmq tutorials" OFF)
if(BUILD_TUTORIALS)
#    add_subdirectory(examples/rabbitmq_tutorials)
endif()

set(AMQP-CPP_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(AMQP-CPP_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

if (WIN32)
    add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN)
endif()
