cmake_minimum_required(VERSION 3.8)
project(mcrecv)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        main.cpp
        MulticastClient.cpp
        MulticastClient.h)

add_executable(mcrecv ${SOURCE_FILES})