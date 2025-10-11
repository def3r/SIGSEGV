#include "dbg.h"

int main() {
  vector<int> v = {1, 2, 3, 4, 5, 6, 7};
  dbg_print_each(v);
  dbg_var_t(v.begin().base());
  dbg_var_t(v.size());
  dbg_var(v.capacity());

  vector<int>::iterator it = v.begin() + 3;
  dbg_var(*it);

  // Resize has happened
  v.push_back(8);
  dbg_nl();
  dbg_print_each(v);
  dbg_var_t(v.begin().base());
  dbg_var_t(v.size());
  dbg_var(v.capacity());

  // iterator is invalidated
  // Storing iterators is foolishness
  dbg_var(*it);

  // no resize on erase
  v.erase(v.begin() + 4);
  v.erase(v.begin() + 1);
  v.erase(v.begin() + 2);
  v.erase(v.begin() + 3);
  dbg_nl();
  dbg_print_each(v);
  dbg_var_t(v.begin().base());
  dbg_var_t(v.size());
  dbg_var(v.capacity());

  v.shrink_to_fit();
  dbg_nl();
  dbg_println("shrink_to_fit()");
  dbg_print_each(v);
  dbg_var_t(v.begin().base());
  dbg_var_t(v.size());
  dbg_var(v.capacity());
}
