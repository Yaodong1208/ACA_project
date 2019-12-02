cmake_minimum_required(VERSION 3.15)
project(LRUCACHE)

set(CMAKE_CXX_STANDARD 14)

add_executable(LRUCACHE LRUcache.h cmake-build-debug/main.cpp)