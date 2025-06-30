cmake_minimum_required(VERSION 3.10)

project(tlog)

add_library(tlog SHARED)
target_sources(tlog PRIVATE tlog.c)

target_include_directories(tlog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})