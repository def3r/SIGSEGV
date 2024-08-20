#include <iostream>

using namespace std;

int main() {
  int T;
  int N, C, Q;
  int L, R;
  int i, j;
  int b;

  cin >> T;
  for (i = 0; i < T; i++) {
    cin >> N >> C >> Q;

    b = C;
    for (j = 0; j < Q; j++) {
      cin >> L >> R;

      if (b >= L && b <=R) {
        b = R - (b - L);
      }
    }

    cout << b << endl;
  }

  return 0;
}
