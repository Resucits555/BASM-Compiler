cmake_minimum_required(VERSION 3.28)
project(BASM-compiler LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


add_executable(main
src/main.cpp
src/headers.cpp
src/textCompiler.cpp)


target_include_directories(main PUBLIC dependencies/mini-rgx/src)
link_directories(build/dependencies/mini-rgx/Debug)
add_subdirectory(dependencies/mini-rgx)
target_link_libraries(main PUBLIC mini-rgx)

target_include_directories(main PUBLIC dependencies/pugixml-1.15/src)
link_directories(build/dependencies/pugixml-1.15/Debug)
add_subdirectory(dependencies/pugixml-1.15)
target_link_libraries(main PUBLIC pugixml)