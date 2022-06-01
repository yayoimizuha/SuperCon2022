cmake_minimum_required(VERSION 3.22)
project(SuperCon2022)

set(CMAKE_CXX_STANDARD 11)

add_executable(SuperCon2022-Pre3 sc3.cpp)
add_executable(SuperCon2022-Pre2 sc2.cpp)
add_executable(SuperCon2022-Pre1 sc1.cpp)

add_subdirectory(sc3)
add_subdirectory(sc2)
add_subdirectory(sc1)
