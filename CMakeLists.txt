cmake_minimum_required(VERSION 3.22)
project(hparena)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-Wall -Wextra -Wpedantic)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

add_library(hparena SHARED src/hparena.cpp)
install(TARGETS hparena)
target_include_directories(hparena PUBLIC include)
