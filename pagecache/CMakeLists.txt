cmake_minimum_required(VERSION 3.10)


option(WITH_DEBUG "Build with debug information" OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")


IF(WITH_DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWITH_DEBUG")
ENDIF()