cmake_minimum_required(VERSION 3.15)
project(SuperCon2022)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Wall -O1")


add_executable(SuperCon2022-Pre3 sc3.cpp)
add_executable(SuperCon2022-Pre2 sc2.cpp)
add_executable(SuperCon2022-Pre1 sc1.cpp)

add_subdirectory(sc3)
add_subdirectory(sc2)
add_subdirectory(sc1)

enable_testing()
add_test(NAME Pre-3 COMMAND $<TARGET_FILE:SuperCon2022-ProblemGenerator-Pre3> 12345 | $<TARGET_FILE:SuperCon2022-Pre3>)
add_test(NAME Pre-2 COMMAND $<TARGET_FILE:SuperCon2022-ProblemGenerator-Pre2> 12345 | $<TARGET_FILE:SuperCon2022-Pre2>)
add_test(NAME Pre-1 COMMAND $<TARGET_FILE:SuperCon2022-ProblemGenerator-Pre1> 12345 0.1 | $<TARGET_FILE:SuperCon2022-Pre1>)

