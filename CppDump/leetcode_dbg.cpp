#include <bits/stdc++.h>
using namespace std;

#define fast_io()                   \
  ios_base::sync_with_stdio(false); \
  cin.tie(NULL)

#define dbg_test_fun

static uint16_t dbg_test_no = 0;

#define dbg_ansi_red "\033[31m"
#define dbg_ansi_green "\033[32m"
#define dbg_ansi_yellow "\033[33m"
#define dbg_ansi_reset "\033[0m"
#define dbg_tab << "\t" <<
#define dbg_newline << "\n" <<

#define dbg(var) cout << #var << ": " << var << "\n"
#define dbg_t(var) cout << #var << ": " << var << "\t"
#define dbg_nl() cout << "\n"

#define dbg_print(...) cout << __VA_ARGS__
#define dbg_println(...) cout << __VA_ARGS__ << "\n"
#define dbg_cprint(...) printf(__VA_ARGS__)
#define dbg_cprintln(...) \
  do {                    \
    printf(__VA_ARGS__);  \
    printf("\n");         \
  } while (0)
#define dbg_print_each(list)       \
  do {                             \
    for (auto listItem : list) {   \
      dbg_print(listItem << ", "); \
    }                              \
    dbg_nl();                      \
  } while (0)
#define dbg_text(color, ...) \
  do {                       \
    printf(color);           \
    dbg_cprint(__VA_ARGS__); \
    printf(dbg_ansi_reset);  \
  } while (0)

#define dbg_error(...) cerr << __VA_ARGS__ << "\n"
#define dbg_panic(...)           \
  do {                           \
    cerr << __VA_ARGS__ << "\n"; \
    exit(EXIT_FAILURE);          \
  } while (0)
#define dbg_cerror(...)           \
  do {                            \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n");        \
  } while (0)
#define dbg_cpanic(...)           \
  do {                            \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n");        \
    exit(EXIT_FAILURE);           \
  } while (0)

#define dbg_test(val)                                         \
  do {                                                        \
    dbg_text(dbg_ansi_yellow, "[TEST #%d]\n", dbg_test_no++); \
    auto dbg_test_ret_val = dbg_test_fun;                     \
    if (dbg_test_ret_val == val) {                            \
      dbg_text(dbg_ansi_green, "Passed\n");                   \
    } else {                                                  \
      dbg_text(dbg_ansi_red, "Failed\n");                     \
      dbg_error("Expected-> " << val);                        \
      dbg_error("Got     -> " << dbg_test_ret_val);           \
    }                                                         \
    dbg_println("---------------------");                     \
  } while (0)
#define dbg_test_with(val, ...) \
  do {                          \
    __VA_ARGS__;                \
    dbg_test(val);              \
  } while (0)

int algo(vector<int>& nums, int k) {
  int idx = 0;
  for (int i : nums) {
    dbg_t(idx);
    dbg(i);
    if (i == k) {
      return idx;
    }
    idx++;
  }
  return -1;
}

#undef dbg_test_fun
int main() {
  vector<int> nums;
  int k;

#define dbg_test_fun algo(nums, k)
  dbg_test_with(3, nums = {1, 2, 3, 4, 5}; k = 4);
  dbg_test_with(-1, k = 7);
  dbg_test_with(1, nums = {69, 420, 17, 300}; k = 420);
}
