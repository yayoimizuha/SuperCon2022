set(CMAKE_C_FLAGS "-DSFMT_MEXP=19937")
add_executable(SuperCon2022-ProblemGenerator-Pre1 gene_sc1.c SFMT-src-1.5.1/SFMT.c)