set(CMAKE_C_FLAGS "-DSFMT_MEXP=19937")
add_executable(SuperCon2022-ProblemGenerator-Pre2 gene_sc2.c SFMT-src-1.5.1/SFMT.c)