add_executable(SuperCon2022-Final main.cpp)
include_directories(sample/team_XXX_MPI)

find_package(MPI REQUIRED)
target_link_libraries(SuperCon2022-Final ${MPI_LIBRARIES})
target_include_directories(SuperCon2022-Final PRIVATE ${MPI_INCLUDE_PATH})