cmake_minimum_required(VERSION 3.8)
project(FTPServer)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES main.cpp)
set(HEADER_FILES )

include_directories(include/)

add_executable(FTPServer ${SOURCE_FILES} ${HEADER_FILES})
