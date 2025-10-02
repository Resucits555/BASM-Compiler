cmake_minimum_required(VERSION 3.28)
project(BASM-compiler LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


add_library(mini-rgx
src/mini-rgx.cpp
src/mini-rgx.h)