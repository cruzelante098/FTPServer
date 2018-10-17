cmake_minimum_required(VERSION 3.6)
project(FTPServer)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES src/ClientConnection.cpp src/MainFTPServerApp.cpp src/FTPServer.cpp src/common.cpp)
set(HEADER_FILES include/ClientConnection.h include/common.h include/FTPServer.h)

include_directories(include/)
find_package (Threads)
add_executable(FTPServer ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries (FTPServer ${CMAKE_THREAD_LIBS_INIT})
