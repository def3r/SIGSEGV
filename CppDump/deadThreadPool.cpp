#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
using namespace std;

mutex mtx;
int threadCount = 0;
const int maxThreads = 20;
condition_variable cv;

void worker(int id) {
  {
    // std::unique_lock, very much like unique_ptrs,
    // when it goes out of scope, the mutex is released
    // and it is unlocked! The mutex is locked as soon as
    // the unique_lock object is created.
    //
    // One advantage of unique_lock is that this lock can
    // be released inside the scope (unlike lock_guard), which
    // is what the condition_variable needs in order to satisfy
    // the condition
    unique_lock<mutex> lock(mtx);

    // condition_variable.wait(), it needs a lock, which it releases
    // in order to satisfy the condition, ones it is satisfied,
    // the mutex is locked again.
    //
    // Suppose the condition is not satisfied, & threadCount's
    // updates depend on the mutex, if cv.wait would keep the mutex
    // locked in order to check for condition, and the condition
    // needs mutex unlocked in order to update threadCount, this will
    // create a deadlock, thus the cv needed to release the lock!
    //
    // And big advantage: Tis no Busy Waiting!
    cv.wait(lock, [] {
      cout << "\rsleepyWaiting";
      return threadCount + 1 < maxThreads;
    });
    cout << "\n";
    threadCount++;
  }

  cout << "Scheduled thread: " << id << "\n";
  this_thread::sleep_for(chrono::seconds(2));

  {
    // std::lock_guard, again, very much like unique_ptrs,
    // when it goes out of scope, the mutex is released
    // and is unlocked! The mutex is locked as soon as
    // the lock_guard object is created. As its name, it
    // locks the mutex upon creation and does not release it
    // until it is deleted (ie goes out of scope)
    lock_guard<mutex> lock(mtx);
    threadCount--;

    // Notify all wakes up the threads that are currently sleeping
    // this makes them recalculate the condition!
    cv.notify_all();
  }
}

int main() {
  cout << "Creating 50 threads!";
  std::vector<std::thread> threads;  // ThreadPool?

  for (int i = 0; i < 50; i++) {
    // Using condition variables is very efficient!
    // This allows us not to use busy waiting
    // and hog the cpu cycles, like a nerd

    threads.emplace_back(worker, i);
    // Also emplace_back is kinda effecient too!
    // 1. Normal push_back in case of an rvalue:
    //      strings.push_back("string");
    //    This will first create a temporary:
    //      string temp = "string";
    //    Now this temporary is moved/copied inside
    //    the vector strings.
    //      strings <- temp
    //
    // 2. In case of emplace_back, all this happens
    //    in place, ie object construction happens
    //    inside the vector,
    //      strings.emplace_back("string");
    //  =>  strings <- std::string("string")
    //    Thus the step to move/copy is not needed
    //    in case of emplace_back
  }

  // Not a spinlock!
  for (auto& thread : threads) {
    thread.join();
  }

  cout << "Threads complete!\n";
}
