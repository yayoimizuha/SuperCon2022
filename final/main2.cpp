#include<bits/stdc++.h>
//#include <mpi.h>  // MPI を使用する場合、このヘッダーを必ず include すること。
#include <omp.h>

//#include "tests/sc_header.h"  // mpi.h の直後に、このヘッダーを必ず include すること。
#include "tests/sc_header.h"  // mpi.h の直後に、このヘッダーを必ず include すること。

//
//こうか
int myid;
// MPI_Comm_size の値が代入される。
int num_procs;
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
// affinities[i][j] = 状態iと状態jの相性(小さいほどよい)
long affinities[sc::N_MAX][sc::N_MAX];

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
        const int BS = (sc::N_MAX + (num_procs - 1)) / num_procs;
        int accs_pre[BS];
        for (int i = myid * BS; i < (myid + 1) * BS && i < sc::n; i++) {
            int q = i;
            for (int k = 0; sc::w[j][k] != '\0'; k++) { q = sc::T[sc::w[j][k] - 'a'][q]; }
            accs_pre[i - myid * BS] = sc::F[q];

        }

        for (int i = 0; i < BS; i++) accs[j][i] = accs_pre[i];

        //memcpy(accs[j],accs_pre,BS);
        //MPI_Allgather(accs_pre, BS, MPI_INT, accs[j], BS, MPI_INT, MPI_COMM_WORLD);
    }
}

void calc_affinities() {
    int count[4];
    for (int i = 0; i < sc::n; i++) {
        for (int j = i + 1; j < sc::n; j++) {
            affinities[i][j] = 0;
            for (int k = 0; k < 4; k++) {
                count[k] = 0;
            }
            for (int k = 0; k < sc::m; k++) {
                int num = accs[k][i] * 2 + accs[k][j];
                count[num]++;
            }
            for (int k = 0; k < 4; k++) {
                affinities[i][j] += count[k] * count[k];
            }
            affinities[j][i] = affinities[i][j];
        }
    }
    for (int i = 0; i < sc::n; i++) {
        affinities[i][i] = sc::m * sc::m;
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

/*
int complete_count[1 << 10][100000];
int c_num[1 << 10];

void calc_all_cant_dist(){
  std::cout << "calc all_cant_dist" << std::endl;
  for(int i = 0; i < 100; i++){
//    std::vector<std::set<int>> complete_count(1 << 10);
    for(int j = 0; j < (1 << 10); j++){
      c_num[j] = 0;
    }
    for(int j = 0; j < sc::m; j++){
      int num = 0;
      for(int k = 0; k < 10; k++){
        num += (accs[j][10*i+k] << k);
      }
      complete_count[num][c_num[num]] = j;
      c_num[num]++;
//      complete_count[num].insert(j);
    }

//    std::set<std::pair<int, int>> next_complete;

    if(i == 0){
      for(int j = 0; j < (1 << 10); j++){
  //      std::set<int> complete_graph = complete_count[j];
        for(int k = 0; k < c_num[j]; k++){
          for(int l = k+1; l < c_num[j]; l++){
            all_cant_dist.insert(std::pair<int, int>(complete_count[j][k], complete_count[j][l]));
          }
        }
      }
    }else{
      for(int j = 0; j < (1 << 10); j++){
  //      std::set<int> complete_graph = complete_count[j];
        for(int k = 0; k < c_num[j]; k++){
          for(int l = k+1; l < c_num[j]; l++){
            all_cant_dist.erase(std::pair<int, int>(complete_count[j][k], complete_count[j][l]));
          }
        }
      }

      std::set<std::pair<int, int>> sub = all_cant_dist;
      auto itr = sub.begin();
      for(int j = 0; j < sub.size(); j++){
        if(next_complete.end() == next_complete.find(*itr)){
          all_cant_dist.erase(*itr);
        }
        itr++;
      }

    }
//    std::cout << "next_complete : " << next_complete.size() << "\t";
  }
  std::cout << "all_cant_dist : " << all_cant_dist.size() << std::endl;
}
*/

//std::set<int, int> get_cant_dist_set(int k, int qs )

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

// 状態 qs[0] から 状態 qs[k-1] のk個の状態を使った時に区別できる文字列のペアの個数を返す関数。
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

// 状態 qs[0] から 状態 qs[k-1] のk個の状態を使った時に区別できない文字列のペアのsetを返す
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


// スコアを計算する関数（ただし、出力が正しいかどうかのチェックはここではしない）。
// int k: 使用する状態の個数を表す。
// int *qs: 使用する状態の情報を持った長さ k の配列。
long get_score(int k, int *qs) {
    long score = k * sc::n * sc::n;
    for (int i = 0; i < k; i++) score += qs[i];
    return score;
}

long get_worst_score() { return 2ll * sc::n * sc::n * sc::n; }

// 使用する状態集合を探索する関数。
void search() {
    const int max_sep_c = get_max_sep_c();

    // 各プロセス毎に異なる乱数シードのもと独立に探索をおこなう。
    xor_shift rand_gen(1 + myid);
    int res_k = sc::n;
    int res_qs[sc::N_MAX];
    for (int i = 0; i < sc::n; i++) res_qs[i] = 1 + i;
    sc::output(res_k, res_qs);

    long res_score = get_score(res_k, res_qs);
    while (sc::get_elapsed_time() + 10.0 < sc::TIME_LIMIT) {
        int k = sc::n;
        int qs[sc::N_MAX];
        for (int i = 0; i < sc::n; i++) qs[i] = 1 + i;
        // random shuffle
        for (int i = 0; i < sc::n - 1; i++) {
            const int j = rand_gen.range_rnd(i, sc::n - 1);
            std::swap(qs[i], qs[j]);
        }
        // get_sep_c(k - 1, qs) == max_sep_c を満たす間、k を減らす。
        while (sc::get_elapsed_time() + 10.0 < sc::TIME_LIMIT) {
            const int sep_c = get_sep_c(k - 1, qs);
            if (sep_c != max_sep_c) break;
            k--;
        }
        long score = get_score(k, qs);

        std::cerr << myid << " " << score << " " << sc::get_elapsed_time() << std::endl;

        if (score < res_score) {
            res_k = k;
            for (int i = 0; i < res_k; i++) res_qs[i] = qs[i];
            res_score = score;
        }
    }

    // 最も小さいコストの解を持つプロセスを求める。
    //struct score_id {
    //    long score;
    //    int id;
    //} cur, mv;
    //cur = {res_score, myid};
    //printf("MPI_Allreduce\n");
    //MPI_Allreduce(&cur, &mv, 1, MPI_LONG_INT, MPI_MINLOC, MPI_COMM_WORLD);
    //if (mv.id != 0) {
    //    if (myid == mv.id) {
    //        std::cerr << "send"
    //                  << " " << myid << std::endl;
    //        MPI_Send(&res_k, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    //        MPI_Send(&res_qs, res_k, MPI_INT, 0, 0, MPI_COMM_WORLD);
    //    } else if (myid == 0) {
    //        MPI_Status status;
    //        std::cerr << "recv"
    //                  << " " << myid << "\n";
    //        MPI_Recv(&res_k, 1, MPI_INT, mv.id, 0, MPI_COMM_WORLD, &status);
    //        MPI_Recv(&res_qs, res_k, MPI_INT, mv.id, 0, MPI_COMM_WORLD, &status);
    //    }
    //}

    sc::output(res_k, res_qs);
}

//状態kを追加するといくつ被覆が増えるか
int get_benefit(int k, std::set<std::pair<int, int>> not_done_pair) {
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

void remove_k(int k, std::set<std::pair<int, int>> not_done_pair) {

}

//ソート用比較関数
bool asc_desc(std::pair<int, int> &left, std::pair<int, int> &right) {
    return right.second < left.second;
}

// 使用する状態集合を探索する関数。
void greedy_search() {
    std::cout << "start greedy" << std::endl;
    const int max_sep_c = get_max_sep_c();

    // 各プロセス毎に異なる乱数シードのもと独立に探索をおこなう。
    xor_shift rand_gen(1 + myid);
    int res_k = sc::n;
    int res_qs[sc::N_MAX];
    for (int i = 0; i < sc::n; i++) res_qs[i] = 1 + i;
    sc::output(res_k, res_qs);
    long res_score = get_score(res_k, res_qs);
    all_cant_dist = get_cant_dist_set(res_k, res_qs);
    std::cout << "All cant dist : " << all_cant_dist.size() << std::endl;

    //選んだ状態
    bool is_done[sc::N_MAX];
    for (int i = 0; i < sc::N_MAX; i++) {
        is_done[i] = false;
    }


    //ある状態がいくつ区別できるか
    std::vector<std::pair<int, int>> states = std::vector<std::pair<int, int>>();
    for (int i = 0; i < sc::n; i++) {
        states.push_back(std::pair<int, int>(i, dist_num[i]));
    }
    std::sort(states.begin(), states.end(), asc_desc);

/*
  //最初の30個
  int k = 0;
  int qs[sc::N_MAX];
  qs[k] = states[0].first + 1;
  is_done[qs[k] - 1] = true;
  k++;
  for(int i = 0; i < 29; i++){
    int min_index = 0;
    long min_affinity = 2000000000l;
    for(int j = 0; j < sc::n; j++){
      if(is_done[j]){
        continue;
      }
      long affinity = 0;
      for(int l = 0; l < k; l++){
        affinity += affinities[j][qs[l]-1];
      }
//      std::cout << min_affinity << " " << affinity << " " << j << std::endl;
      if(affinity < min_affinity || min_affinity == 2000000000l){
        min_affinity = affinity;
        min_index = j;
      }
    }
    qs[k] = min_index + 1;
    std::cout << qs[k] << std::endl;
    is_done[qs[k] - 1] = true;
    k++;
  }
*/

    //最初の30個
    int k = 0;
    int qs[sc::N_MAX];
    for (int i = 0; i < 50; i++) {
        qs[k] = states[i].first + 1;
        is_done[qs[k]] = true;
        k++;
    }

    int old_k = sc::n;
    int old_qs[sc::N_MAX];
    for (int i = 0; i < k; i++) {
        old_qs[i] = qs[i];
    }

    std::set<std::pair<int, int>> not_done_pair = get_cant_dist_set(k, qs);
    auto itr = all_cant_dist.begin();
    for (int i = 0; i < all_cant_dist.size(); i++) {
        not_done_pair.erase(*itr);
        itr++;
    }
    std::cout << "not_done_pair :" << not_done_pair.size() << std::endl;

    long score = get_score(k, qs);

    std::cerr << myid << " " << score << " " << sc::get_elapsed_time() << std::endl;


    while (sc::get_elapsed_time() - 6000.0 < sc::TIME_LIMIT) {
        while (true) {
            if (not_done_pair.size() == 0) {
                break;
            }
            int max_index = 0;
            int max_benefit = -1;
            int max_benefits[nthreads];
            int max_indexs[nthreads];
//      std::cout << std::endl << "@@@" << std::endl;
#pragma omp parallel
            {
                int ithread = omp_get_thread_num();
                max_benefits[ithread] = -1;
		int benefit;

#pragma omp for private(benefit)
		for (int i = 0; i < sc::N_MAX; i++) {
                    if (is_done[i]) {
                        continue;
                    }
                    benefit = get_benefit(i, not_done_pair);
                    if (benefit > max_benefits[ithread] && benefit > 0) {
                        max_benefits[ithread] = benefit;
                        max_indexs[ithread] = i;
                    }
//          if(ithread == 17){
//            std::cout << "[" << ithread << "]" << std::endl;
//          }
                }
//        #pragma omp critical
                {
//          std::cout << "[" << omp_get_thread_num() << "]"  << temp_max_benefit << std::endl;
//          if(max_benefit < temp_max_benefit){
                    //           max_benefit = temp_max_benefit;
//            max_index = temp_max_index;
//          }
                }
//        #pragma omp critical
//        std::cout << "[" << omp_get_thread_num() << "]" << std::endl;
            }

            for (int i = 0; i < nthreads; i++) {
                if (max_benefit < max_benefits[i]) {
                    max_benefit = max_benefits[i];
                    max_index = max_indexs[i];
                }
            }

            //    std::cout << not_done_pair.size() << ",";
            //    std::cout << std::endl << "###" << std::endl;

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

            //    std::cout << "not_done_pair :" << not_done_pair.size() << std::endl;

            qs[k] = max_index + 1;
            k++;
            if (k > old_k + 1) {
                break;
            }
        }

        score = get_score(k, qs);

        //std::cout << std::endl;
        if (score < res_score) {
            res_k = k;
            for (int i = 0; i < res_k; i++) res_qs[i] = qs[i];
            res_score = score;
            std::cout << std::endl  << "*Best* " << res_score << std::endl;
	    system("date");
            sc::output(res_k, res_qs);
            old_k = k;
            for (int i = 0; i < k; i++) {
                old_qs[i] = qs[i];
            }
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
        //printf("search_with: %d\n", search_width);
        k -= search_width;
        not_done_pair = get_cant_dist_set(k, qs);
    }


    // 最も小さいコストの解を持つプロセスを求める。
    //struct score_id {
    //    long score;
    //    int id;
    //} cur, mv;
    //cur = {res_score, myid};
    //MPI_Allreduce(&cur, &mv, 1, MPI_LONG_INT, MPI_MINLOC, MPI_COMM_WORLD);
    //if (mv.id != 0) {
    //    if (myid == mv.id) {
    //        std::cerr << "send"
    //                  << " " << myid << std::endl;
    //        MPI_Send(&res_k, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    //        MPI_Send(&res_qs, res_k, MPI_INT, 0, 0, MPI_COMM_WORLD);
    //    } else if (myid == 0) {
    //        MPI_Status status;
    //        std::cerr << "recv"
    //                  << " " << myid << "\n";
    //        MPI_Recv(&res_k, 1, MPI_INT, mv.id, 0, MPI_COMM_WORLD, &status);
    //        MPI_Recv(&res_qs, res_k, MPI_INT, mv.id, 0, MPI_COMM_WORLD, &status);
    //    }
    //}

    sc::output(res_k, res_qs);
}

// main関数で入力を読み込んだ後、以下の関数が実行される。
void run() {
    std::cout << "Bar!" << std::endl;
    //MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    num_procs = 1;
    //MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    myid = 0;
    for (int c = 0; c < sc::A_SIZE; c++)
        for (int i = 0; i < sc::n; i++) sc::T[c][i]--;
    simulate();
    std::cout << "Fizz!" << std::endl;
//  calc_dist();
    calc_dist_num();
//  calc_all_cant_dist();
//  return;
    std::cout << "Buzz!" << std::endl;
//  calc_affinities();
    std::cout << "max_sep_c : " << get_max_sep_c() << std::endl;
    std::cerr << myid << " simulated " << sc::get_elapsed_time() << std::endl;
    std::cout << "FizzBuzz!" << std::endl;
    greedy_search();
}

int main(int argc, char **argv) {
    std::cout << "Hello!" << std::endl;
    sc::initialize(argc, argv);  // はじめに sc::initialize(argc, argv) を必ず呼び出すこと。
    std::cout << "Foo!" << std::endl;
    run();
    sc::finalize();  // おわりに sc::finalize() を必ず呼び出すこと。
}

//oo



