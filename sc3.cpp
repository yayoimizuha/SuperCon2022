#include "sc3/sc3.h"
#include <cstdlib>
#include <cstdio>

int main(int argc, char *argv[]) {
    SC_input();
    printf("%d\n", sizeof(SC_C) / sizeof(int));
    printf("%d\n", sizeof(SC_Cr) / sizeof(int));
}