#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
using namespace std;

const int maxThreads = 20;
int threadCount = 0;
mutex mtx;

void worker(int id) {
  mtx.lock();
  threadCount++;
  mtx.unlock();

  cout << "Scheduled thread: " << id << "\n";
  this_thread::sleep_for(chrono::seconds(2));

  mtx.lock();
  threadCount--;
  mtx.unlock();
}

int main() {
  cout << "Creating 50 threads!";
  std::vector<std::thread> threads;
  for (int i = 0; i < 50; i++) {
    cout << threadCount << "\n";
    while (threadCount + 1 >= maxThreads) {
      cout << "\rBusywaiting";
    }
    threads.emplace_back(worker, i);
  }

  for (auto& thread : threads) {
    thread.join();
  }

  cout << "Threads complete!\n";
}
