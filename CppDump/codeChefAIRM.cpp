#include <iostream>
#include <unordered_map>
// #include <vector>
using namespace std;

int main() {
  int t;
  int n;
  int schedule;

  cin >> t;

  for (int i = 0; i < t; i++) {
    cin >> n;

    int highestFreq = 1;
    unordered_map<int, int> freqMap;

    // arrival
    for (int j = 0; j < 2 * n; j++) {
      cin >> schedule;

      if (++freqMap[schedule] > highestFreq) {
        highestFreq = freqMap[schedule];
      }
    }

    cout << highestFreq << endl;
  }

  return 0;
}
