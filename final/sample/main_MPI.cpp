#include <bits/stdc++.h>
#include <mpi.h>  // MPI を使用する場合、このヘッダーを必ず include すること。

#include "tests/sc_header.h"  // mpi.h の直後に、このヘッダーを必ず include すること。

int myid;
// MPI_Comm_size の値が代入される。
int num_procs;

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
                << " " << myid << std::endl;
      MPI_Recv(&res_k, 1, MPI_INT, mv.id, 0, MPI_COMM_WORLD, &status);
      MPI_Recv(&res_qs, res_k, MPI_INT, mv.id, 0, MPI_COMM_WORLD, &status);
    }
  }

  sc::output(res_k, res_qs);
}

// main関数で入力を読み込んだ後、以下の関数が実行される。
void run() {
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  for (int c = 0; c < sc::A_SIZE; c++)
    for (int i = 0; i < sc::n; i++) sc::T[c][i]--;
  simulate();
  std::cerr << myid << " simulated " << sc::get_elapsed_time() << std::endl;
  search();
}

int main(int argc, char** argv) {
  sc::initialize(argc, argv);  // はじめに sc::initialize(argc, argv) を必ず呼び出すこと。
  run();
  sc::finalize();  // おわりに sc::finalize() を必ず呼び出すこと。
}
