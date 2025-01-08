#include <cstdlib>
#include <iostream>
#include <strings.h>
#include <unordered_map>
#include <vector>

using namespace std;

int main() {
// CLEARANCE
  int X;

  cin >> X;
  cout <<  X + (X / 2);

// CAKEHALF
  int T;
  pair<int, int> p;
  int totalEaten = 0;
  int temp;

  cin >> T;

  for (int i = 0; i < T; i++) {
    cin >> p.first >> p.second;

    while (p.first != p.second) {
      if (p.first > p.second) {
        temp = (p.first / 2) + (p.first % 2);

        totalEaten += temp;
        p.first -= temp;
      } else {
        temp = (p.second / 2) + (p.second % 2);
        totalEaten += temp;
        p.second -= temp;
      }
    }

    cout << totalEaten << endl;
    totalEaten = 0;
  }

// FIGBOT

  int T;
  int N;
  pair<int, int> alice;
  pair<int, int> bob;
  string moves;

  cin >> T;
  for (int i = 0; i < T; i++) {
    alice.first = 0; alice.second = 0;

    cin >> N >> bob.first >> bob.second;
    cin >> moves;

    int flag = 0;

    for (int n = 0; n < N; n++) {
      switch (moves.at(n)) {
        case 'R':
          alice.first++;
          break;

        case 'L':
          alice.first--;
          break;

        case 'U':
          alice.second++;
          break;

        case 'D':
          alice.second--;
          break;
      }

      if (abs(bob.first - alice.first) + abs(bob.second - alice.second) == n + 1) {
        flag = 1;
        break;
      }
    }

    cout << (flag ? "Yes" : "No") << endl;
  }


// JUSTTWO

  int T;
  int N;

  cin >> T;
  for(int i = 0; i < T; i++) {
    cin >> N;

    vector<int> a(N);
    unordered_map<int, int> freqMap;

    for (int indx = 0; indx< N; indx++) {
      cin >> a[indx];
      freqMap[a[indx]]++;
    }

    // O(n^2) complexity but wateber
    for (int x = 0; x < N; x++) {
      for (int y = 0; y < N; y++) {
        // bruh
      }
    }
  }

  return 0;
}
