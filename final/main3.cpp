#include<bits/stdc++.h>
//#include <mpi.h>  // MPI を使用する場合、このヘッダーを必ず include すること。
#include <omp.h>

#include "tests/sc_header.h"  // mpi.h の直後に、このヘッダーを必ず include すること。

int nthreads = omp_get_max_threads();

/*---------定義---------*/

// ペアの数(定数)
constexpr int PAIR_NUM = (sc::M_MAX * (sc::M_MAX - 1)) / 2;
// can_dist[i][j] = 状態iにおいてペアjが区別できるか
bool can_dist[sc::N_MAX][PAIR_NUM];
// dist_num[i] = 状態iのみで区別できるペアの数
int dist_num[sc::N_MAX];
// all_dist[i] = 初期状態を全部使ったときにペアiが区別可能
bool all_dist[PAIR_NUM];
std::set<std::pair<int, int>> all_cant_dist;
// 使ったcan_distを格納するset
std::set<bool> tmp;

/*--------正規乱数-----*/

int search_width;
std::random_device seed_gen;
std::default_random_engine engine(seed_gen());
std::normal_distribution<> distribution(0, 4.0);
/*-------ここまで-------*/

// 疑似乱数の生成に使用する（これ以外の疑似乱数を用いてもよい）。
struct xor_shift {
    unsigned long long x;

    xor_shift(unsigned long long SEED) { x = SEED; }

    unsigned long long next() {
        x = x ^ (x << 13);
        x = x ^ (x >> 7);
        x = x ^ (x << 17);
        return x;
    }

    // [0, x)
    unsigned long long rnd(unsigned long long x) { return next() % x; }

    // [l, r]
    unsigned long long range_rnd(unsigned long long l, unsigned long long r) { return l + rnd(r - l + 1); }
};

// accs[j][i] = 状態 i からはじめて文字列 w_j の遷移関数で遷移した先の状態が受理状態の場合 1、受理状態でない場合 0
// が代入される。
int accs[sc::M_MAX][sc::N_MAX + 4 * 48];

// オートマトンの遷移関数をシミュレーションする関数
void simulate() {
    for (int j = 0; j < sc::m; j++) {
        const int BS = sc::N_MAX;
        int accs_pre[BS];
        for (int i = 0; i < BS && i < sc::n; i++) {
            int q = i;
            for (int k = 0; sc::w[j][k] != '\0'; k++) { q = sc::T[sc::w[j][k] - 'a'][q]; }
            accs_pre[i] = sc::F[q];

        }

        for (int i = 0; i < BS; i++) accs[j][i] = accs_pre[i];

    }
}

void calc_dist_num() {
    for (int i = 0; i < sc::N_MAX; i++) {
        int count = 0;
        for (int j = 0; j < sc::M_MAX; j++) {
            count += accs[j][i];
        }
        dist_num[i] = count * (sc::M_MAX - count);
    }
}

void calc_dist() {
    for (int i = 0; i < sc::N_MAX; i++) {
        all_dist[i] = false;
        dist_num[i] = 0;
    }
    for (int i = 0; i < sc::N_MAX; i++) {
        int ary_count = 0;
        for (int j = 0; j < sc::M_MAX; j++) {
            for (int k = j + 1; k < sc::M_MAX; k++) {
                can_dist[i][ary_count] = accs[j][i] ^ accs[k][i];
                all_dist[ary_count] |= can_dist[i][ary_count];
                dist_num[i] += can_dist[i][ary_count] ? 1 : 0;
                ary_count++;
            }
        }
    }
}


int get_sep_c(int k, int *qs) {
    int c = 0;
    for (int j1 = 0; j1 < sc::m; j1++) {
        for (int j2 = j1; j2 < sc::m; j2++) {
            bool ok = false;
            for (int i = 0; i < k; i++) {
                if (accs[j1][qs[i] - 1] != accs[j2][qs[i] - 1]) {
                    ok = true;
                    break;
                }
            }
            c += ok;
        }
    }
    return c;
}


std::set<std::pair<int, int>> get_cant_dist_set(int k, int *qs) {
    std::set<std::pair<int, int>> ans;
    for (int j1 = 0; j1 < sc::m; j1++) {
        for (int j2 = j1 + 1; j2 < sc::m; j2++) {
            bool ok = false;
            for (int i = 0; i < k; i++) {
                if (accs[j1][qs[i] - 1] != accs[j2][qs[i] - 1]) {
                    ok = true;
                    break;
                }
            }
            if (!ok && all_cant_dist.end() == all_cant_dist.find(std::pair<int, int>(j1, j2))) {
                ans.insert(std::pair<int, int>(j1, j2));
            }
        }
    }
    return ans;
}

// 全ての状態を使用する場合に区別できる文字列のペアの個数を返す関数。
int get_max_sep_c() {
    int qs[sc::N_MAX];
    for (int i = 0; i < sc::n; i++) qs[i] = 1 + i;
    return get_sep_c(sc::n, qs);
}


long get_score(int k, int *qs) {
    long score = k * sc::n * sc::n;
    for (int i = 0; i < k; i++) score += qs[i];
    return score;
}

long get_worst_score() { return 2ll * sc::n * sc::n * sc::n; }


//状態kを追加するといくつ被覆が増えるか
int get_benefit(int k, const std::set<std::pair<int, int>> &not_done_pair) {
    int result = 0;
    auto itr = not_done_pair.begin();
    for (int i = 0; i < not_done_pair.size(); i++) {
        int j1 = (*itr).first;
        int j2 = (*itr).second;
        result += accs[j1][k] ^ accs[j2][k];
        itr++;
    }
    return result;
}

//ソート用比較関数
bool asc_desc(std::pair<int, int> &left, std::pair<int, int> &right) {
    return right.second < left.second;
}

// 使用する状態集合を探索する関数。
void greedy_search() {
//    std::cout << "start greedy" << std::endl;
    const int max_sep_c = get_max_sep_c();

    // 各プロセス毎に異なる乱数シードのもと独立に探索をおこなう。
    xor_shift rand_gen(1);
    int res_k = sc::n;
    int res_qs[sc::N_MAX];
    for (int i = 0; i < sc::n; i++) res_qs[i] = 1 + i;
    sc::output(res_k, res_qs);
    long res_score = get_score(res_k, res_qs);
    all_cant_dist = get_cant_dist_set(res_k, res_qs);
//    std::cout << "All cant dist : " << all_cant_dist.size() << std::endl;

    //選んだ状態
    bool is_done[sc::N_MAX];
    for (int i = 0; i < sc::N_MAX; i++) {
        is_done[i] = false;
    }


    //ある状態がいくつ区別できるかでソート
    std::vector<std::pair<int, int>> states = std::vector<std::pair<int, int>>();
    for (int i = 0; i < sc::n; i++) {
        states.push_back(std::pair<int, int>(i, dist_num[i]));
    }
    std::sort(states.begin(), states.end(), asc_desc);


    //最初の50個
    int k = 0;
    int qs[sc::N_MAX];
    for (int i = 0; i < 50; i++) {
        qs[k] = states[i].first + 1;
        is_done[qs[k]] = true;
        k++;
    }

    int old_k = sc::n;
    int old_qs[sc::N_MAX];
    for (int i = 0; i < k; i++) old_qs[i] = qs[i];

    std::set<std::pair<int, int>> not_done_pair = get_cant_dist_set(k, qs);
    auto itr = all_cant_dist.begin();
    for (int i = 0; i < all_cant_dist.size(); i++) {
        not_done_pair.erase(*itr);
        itr++;
    }
//    std::cout << "not_done_pair :" << not_done_pair.size() << std::endl;

    long score = get_score(k, qs);

    //std::cerr << myid << " " << score << " " << sc::get_elapsed_time() << std::endl;


    while (sc::get_elapsed_time() - 0.0 < sc::TIME_LIMIT) {
        while (true) {
            if (not_done_pair.size() == 0) {
                break;
            }
            int max_index = 0;
            int max_benefit = -1;
            int max_benefits[nthreads];
            int max_indexs[nthreads];

#pragma omp parallel
            {
                int ithread = omp_get_thread_num();
                xor_shift irand_gen(ithread);
                max_benefits[ithread] = -1;
                int benefit;
                int same_num;

#pragma omp for
                for (int i = 0; i < sc::N_MAX; i++) {
                    if (is_done[i]) continue;

                    benefit = get_benefit(i, not_done_pair);
                    if (benefit > max_benefits[ithread] && benefit > 0) {
                        same_num = 1;
                        max_benefits[ithread] = benefit;
                        max_indexs[ithread] = i;
                    } else if (benefit == max_benefits[ithread]) {
                        same_num++;
                        if (irand_gen.rnd(same_num) == 0) {
                            max_indexs[ithread] = i;
                        }
                    }
                }
            }

            for (int i = 0; i < nthreads; i++) {
                if (max_benefit < max_benefits[i]) {
                    max_benefit = max_benefits[i];
                    max_index = max_indexs[i];
                }
            }

            if (max_benefit == -1) {
                break;
            }

            auto sub = not_done_pair;
            auto itr = sub.begin();
            for (int i = 0; i < sub.size(); i++) {
                int j1 = (*itr).first;
                int j2 = (*itr).second;
                if (accs[j1][max_index] != accs[j2][max_index]) {
                    not_done_pair.erase(*itr);
                }
                itr++;
            }

            qs[k] = max_index + 1;
            k++;
            if (k > old_k + 1) {
                break;
            }
        }

        score = get_score(k, qs);


        if (score < res_score) {
            res_k = k;
            for (int i = 0; i < res_k; i++) res_qs[i] = qs[i];
            res_score = score;
//            std::cout << std::endl << "*Best* " << res_score << std::endl;
            // system("date");
            sc::output(res_k, res_qs);
            old_k = k;
            for (int i = 0; i < k; i++) {
                old_qs[i] = qs[i];
            }
        } else if (k <= old_k) {
//            std::cout << "-slide- " << score << ", k = " << k;
        } else {
            k = old_k;
            for (int i = 0; i < k; i++) {
                qs[i] = old_qs[i];
            }
        }

        for (int i = 0; i < k - 1; i++) {
            const int j = rand_gen.range_rnd(i, k - 1);
            std::swap(qs[i], qs[j]);
        }
        double tmp_with_result = distribution(engine);
        search_width = 1 + (int) (std::abs(tmp_with_result) * 1.7);
//        printf("search_width: %d\n", search_width);
        k -= search_width;
        not_done_pair = get_cant_dist_set(k, qs);
    }

    sc::output(res_k, res_qs);
}

// main関数で入力を読み込んだ後、以下の関数が実行される。
void run() {
//    std::cout << "Bar!" << std::endl;
    for (int c = 0; c < sc::A_SIZE; c++)
        for (int i = 0; i < sc::n; i++) sc::T[c][i]--;
    simulate();
//    std::cout << "Fizz!" << std::endl;
    calc_dist_num();
//    std::cout << "Buzz!" << std::endl;
//    std::cout << "max_sep_c : " << get_max_sep_c() << std::endl;
    //std::cerr << myid << " simulated " << sc::get_elapsed_time() << std::endl;
//    std::cout << "FizzBuzz!" << std::endl;
    greedy_search();
}

int main(int argc, char **argv) {
//    std::cout << "Hello!" << std::endl;
    sc::initialize(argc, argv);  // はじめに sc::initialize(argc, argv) を必ず呼び出すこと。
//    std::cout << "Foo!" << std::endl;
    run();
    sc::finalize();  // おわりに sc::finalize() を必ず呼び出すこと。
}
