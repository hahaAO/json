cmake_minimum_required(VERSION 3.20)
project(vgd_json)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "-O2")

add_executable(json_test main.cpp json.h json.cpp ../json-develop/single_include/nlohmann/json.hpp)

target_precompile_headers(json_test PRIVATE pch.h)