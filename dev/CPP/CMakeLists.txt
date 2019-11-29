cmake_minimum_required(VERSION 3.15)
project(HLB)

set(CMAKE_CXX_STANDARD 11)

add_executable(HLB main.cpp ../HLB/inst_simulate.h hlb_api.h tests/test001.h tests/test002.h)