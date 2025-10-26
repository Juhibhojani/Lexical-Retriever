cmake_minimum_required(VERSION 3.10)
project(DocumentServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(civetweb STATIC
    civetweb/src/civetweb.c         # C API
    civetweb/src/CivetServer.cpp    # C++ wrapper
)

# CivetWeb include directory
target_include_directories(civetweb PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/civetweb/include
)

target_compile_definitions(civetweb PUBLIC CIVETWEB_NO_SSL)


file(GLOB_RECURSE SERVER_SOURCES
    src/*.cpp
    src/controller/*.cpp
    src/service/*.cpp
    src/repository/*.cpp
    src/utils/*.cpp
)


add_executable(server ${SERVER_SOURCES})

target_include_directories(server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/civetweb/include
    /usr/include/postgresql
    /usr/include/nlohmann
)

target_link_libraries(server PRIVATE
    civetweb
    /usr/lib/x86_64-linux-gnu/libpq.so
    pthread
)

