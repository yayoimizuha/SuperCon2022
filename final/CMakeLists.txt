find_package(MPI REQUIRED)

find_package(OpenMP)
if (OpenMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif ()
set(CMAKE_CXX_FLAGS "-mcmodel=medium -fpic -dynamic -fopenmp")

include_directories(sample/team_XXX_MPI SuperCon2022-Final)
add_executable(SuperCon2022-Final main.cpp)
target_link_libraries(SuperCon2022-Final ${MPI_LIBRARIES})
target_include_directories(SuperCon2022-Final PUBLIC ${MPI_INCLUDE_PATH})


find_package(OpenMP)
if (OpenMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif ()

include_directories(sample/team_XXX_MPI SuperCon2022-Final_2)
add_executable(SuperCon2022-Final_2 main2.cpp)
target_link_libraries(SuperCon2022-Final_2 ${MPI_LIBRARIES})
target_include_directories(SuperCon2022-Final_2 PUBLIC ${MPI_INCLUDE_PATH})