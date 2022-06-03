#include "sc3/sc3.h"

int move_up(int n);

int move_down(int n);

int move_left(int n);

int move_right(int n);


int main() {
    SC_input();

    int pos = ((SC_L - 1) / 2) * SC_L + (SC_L - 1) / 2, tmp_pos;
    SC_ans[0] = pos;

    for (int n = 1; n <= SC_Nt; ++n) {


        if (SC_C[tmp_pos = move_up(pos)] == SC_Cr[n]) {
            SC_ans[n] = pos = tmp_pos;
            continue;
        }
        if (SC_C[tmp_pos = move_left(pos)] == SC_Cr[n]) {
            SC_ans[n] = pos = tmp_pos;
            continue;
        }
        if (SC_C[tmp_pos = move_down(pos)] == SC_Cr[n]) {
            SC_ans[n] = pos = tmp_pos;
            continue;
        }
        SC_ans[n] = pos = move_right(pos);
    }
    SC_output();

    return 0;
}


int move_up(int n) {
    n += 11;
    if (n >= SC_N) {
        return n - SC_N;
    }
    return n;
}

int move_down(int n) {
    n -= 11;
    if (n < 0) {
        return n + SC_N;
    }
    return n;
}

int move_left(int n) {
    if (!(n % SC_L))return n + SC_L - 1;
    return n - 1;
}

int move_right(int n) {
    if (!(++n % SC_L))return n - SC_L;
    return n;
}
