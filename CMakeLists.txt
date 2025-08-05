cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)

add_executable(main 
src/main.cpp
src/headers.cpp
src/textCompiler.cpp)

target_compile_features(main PRIVATE cxx_std_17)