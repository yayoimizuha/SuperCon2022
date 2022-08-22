#include <bits/stdc++.h>
#include <mpi.h>  // MPI を使用する場合、このヘッダーを必ず include すること。
#include <omp.h>

#include "tests/sc_header.h"  // mpi.h の直後に、このヘッダーを必ず include すること。

int myid;
// MPI_Comm_size の値が代入される。
int num_procs;

/*---------定義---------*/

// ペアの数(定数)
constexpr int PAIR_NUM = (sc::M_MAX * (sc::M_MAX - 1)) / 2;
// can_dist[i][j] = 状態iにおいてペアjが区別できるか
bool can_dist[sc::N_MAX][PAIR_NUM];
// dist_num[i] = 状態iのみで区別できるペアの数
int dist_num[sc::N_MAX];
// all_dist[i] = 初期状態を全部使ったときにペアiが区別可能
bool all_dist[PAIR_NUM];
// 使ったcan_distを格納するset
std::set<bool> tmp;

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
    MPI_Allgather(accs_pre, BS, MPI_INT, accs[j], BS, MPI_INT, MPI_COMM_WORLD);
  }
}

void calc_dist_num(){
  for(int i = 0; i < sc::N_MAX; i++){
    int count = 0;
    for(int j = 0; j < sc::M_MAX; j++){
      count += accs[j][i];
    }
    dist_num[i] = count * (sc::M_MAX - count);
  }
}

void calc_all_can_dist(){
    for(int i = 0; i < sc::N_MAX; i++){
        all_dist[i] = false;
        dist_num[i] = 0;
    }
    for(int i = 0; i < sc::N_MAX; i++){
        int ary_count = 0;
        for(int j = 0; j < sc::M_MAX; j++){
            for(int k = j+1; k < sc::M_MAX; k++){
                can_dist[i][ary_count] = accs[j][i] ^ accs[k][i];
                all_dist[ary_count] |= can_dist[i][ary_count];
                dist_num[i] += can_dist[i][ary_count] ? 1 : 0;
                ary_count++;
            }
        }
    }
}

void calc_dist() {
    for(int i = 0; i < sc::N_MAX; i++){
        all_dist[i] = false;
        dist_num[i] = 0;
    }
    for(int i = 0; i < sc::N_MAX; i++){
        int ary_count = 0;
        for(int j = 0; j < sc::M_MAX; j++){
            for(int k = j+1; k < sc::M_MAX; k++){
                can_dist[i][ary_count] = accs[j][i] ^ accs[k][i];
                all_dist[ary_count] |= can_dist[i][ary_count];
                dist_num[i] += can_dist[i][ary_count] ? 1 : 0;
                ary_count++;
            }
        }
    }
}

// 状態 qs[0] から 状態 qs[k-1] のk個の状態を使った時に区別できる文字列のペアの個数を返す関数。
int get_sep_c(int k, int* qs) {
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
// 全ての状態を使用する場合に区別できる文字列のペアの個数を返す関数。
int get_max_sep_c() {
  int qs[sc::N_MAX];
  for (int i = 0; i < sc::n; i++) qs[i] = 1 + i;
  return get_sep_c(sc::n, qs);
}

// スコアを計算する関数（ただし、出力が正しいかどうかのチェックはここではしない）。
// int k: 使用する状態の個数を表す。
// int *qs: 使用する状態の情報を持った長さ k の配列。
long get_score(int k, int* qs) {
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
  struct score_id {
    long score;
    int id;
  } cur, mv;
  cur = {res_score, myid};
  MPI_Allreduce(&cur, &mv, 1, MPI_LONG_INT, MPI_MINLOC, MPI_COMM_WORLD);
  if (mv.id != 0) {
    if (myid == mv.id) {
      std::cerr << "send"
                << " " << myid << std::endl;
      MPI_Send(&res_k, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
      MPI_Send(&res_qs, res_k, MPI_INT, 0, 0, MPI_COMM_WORLD);
    } else if (myid == 0) {
      MPI_Status status;
      std::cerr << "recv"
                << " " << myid << "\n";
      MPI_Recv(&res_k, 1, MPI_INT, mv.id, 0, MPI_COMM_WORLD, &status);
      MPI_Recv(&res_qs, res_k, MPI_INT, mv.id, 0, MPI_COMM_WORLD, &status);
    }
  }

  sc::output(res_k, res_qs);
}

//状態kを追加するといくつ被覆が増えるか
//O(M^2)
int get_benefit(int k, std::set<int> not_done_pair){
  int result = 0;
  for(int i = 0; i < PAIR_NUM; i++)
    result += (can_dist[k][i] && *not_done_pair.find(i)) ? 1 : 0;
  return result;
}

//ソート用比較関数
bool asc_desc(std::pair<int, int>& left, std::pair<int, int>& right) {
  return right.second < left.second;
}

// 使用する状態集合を探索する関数。
void greedy_search() {
  std::cout << "start greedy" << std::endl;
  const int max_sep_c = get_max_sep_c();

  std::vector<std::pair<int, int>> states = std::vector<std::pair<int, int>>();
  for(int i = 0; i < sc::n; i++){
    states.push_back(std::pair<int, int>(i, dist_num[i]));
  }

  std::sort(states.begin(), states.end(), asc_desc);

  // 各プロセス毎に異なる乱数シードのもと独立に探索をおこなう。
  xor_shift rand_gen(1 + myid);
  int res_k = sc::n;
  int res_qs[sc::N_MAX];
  for (int i = 0; i < sc::n; i++) res_qs[i] = 1 + i;
  sc::output(res_k, res_qs);

  long res_score = get_score(res_k, res_qs);

//  while (sc::get_elapsed_time() + 50.0 < sc::TIME_LIMIT) {
    std::set<int> not_done_pair;
    for(int i = 0; i < PAIR_NUM; i++){
      if(all_dist[i]){
        not_done_pair.insert(i);
      }
    }
    std::cout << "All can dist : " << not_done_pair.size() << std::endl;

    int k = 0;
    int qs[sc::N_MAX];
    for(int i = 0; i < 30; i++){
      std::cout << (states[i].second) << std::endl;
      for(int i = 0; i < PAIR_NUM; i++){
        if(can_dist[k][i]){
          not_done_pair.erase(i);
        }
      }
    }


//    while (sc::get_elapsed_time() + 10.0 < sc::TIME_LIMIT) {
    while (true) {
      int max_benefit = 0;
      int max_index = 0;
      int benefit;
      bool is_max;

      for(int i = 0; i < 30; i++){
        for(int j = 0; j < PAIR_NUM; j++){
          if(can_dist[states[i].first][j]){
            not_done_pair.erase(j);
          }
        }
      }

      std::cout << "not yet can dist : " << not_done_pair.size() << std::endl;
      return;

      if(max_benefit == 0){
        break;
      }
      qs[k] = max_index;
      k++;

      for(int i = 0; i < PAIR_NUM; i++){
        if(can_dist[k][i]){
          not_done_pair.erase(i);
        }
      }
      std::cout << "@" << k << std::endl;
    }
    long score = get_score(k, qs);

    std::cerr << myid << " " << score << " " << sc::get_elapsed_time() << std::endl;

    if (score < res_score) {
      res_k = k;
      for (int i = 0; i < res_k; i++) res_qs[i] = qs[i];
      res_score = score;
    }
//  }

  // 最も小さいコストの解を持つプロセスを求める。
  struct score_id {
    long score;
    int id;
  } cur, mv;
  cur = {res_score, myid};
  MPI_Allreduce(&cur, &mv, 1, MPI_LONG_INT, MPI_MINLOC, MPI_COMM_WORLD);
  if (mv.id != 0) {
    if (myid == mv.id) {
      std::cerr << "send"
                << " " << myid << std::endl;
      MPI_Send(&res_k, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
      MPI_Send(&res_qs, res_k, MPI_INT, 0, 0, MPI_COMM_WORLD);
    } else if (myid == 0) {
      MPI_Status status;
      std::cerr << "recv"
                << " " << myid << "\n";
      MPI_Recv(&res_k, 1, MPI_INT, mv.id, 0, MPI_COMM_WORLD, &status);
      MPI_Recv(&res_qs, res_k, MPI_INT, mv.id, 0, MPI_COMM_WORLD, &status);
    }
  }

  sc::output(res_k, res_qs);
}

// main関数で入力を読み込んだ後、以下の関数が実行される。
void run() {
  std::cout << "Bar!" << std::endl;
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  for (int c = 0; c < sc::A_SIZE; c++)
    for (int i = 0; i < sc::n; i++) sc::T[c][i]--;
  simulate();
  std::cout << "Fizz!" << std::endl;
  calc_dist();
  calc_dist_num();
  std::cout << "Buzz!" << std::endl;
  std::cerr << myid << " simulated " << sc::get_elapsed_time() << std::endl;
  std::cout << "FizzBuzz!" << std::endl;
  greedy_search();
}

int main(int argc, char** argv) {
  std::cout << "Hello!" << std::endl;
  sc::initialize(argc, argv);  // はじめに sc::initialize(argc, argv) を必ず呼び出すこと。
  std::cout << "Foo!" << std::endl;
  run();
  sc::finalize();  // おわりに sc::finalize() を必ず呼び出すこと。
}


