#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

using namespace std;

// Synchronizing Variable
// Thread safe + no locking required
atomic<bool> running = true;

void blockCount() {
  for (long i = 0; i < 9999999999 * 2; i++) {
  }
  running = false;
}

void ioWait() {
  float i = 0;
  while (running) {
    i += 0.1;
    if (i > 4)
      i = 0;
    cout << "\033[2K\r";
    // So this flush is required to display on screen immediately!
    // The standard output is nothin but a buffer
    // The cout "buffering" prevents it from displaying the output
    // in real time.
    // Thus, we need to manually flush the buffer to see it real time
    cout << "Waiting" << flush;
    for (int j = 0; j < (int)i; j++) {
      cout << "." << flush;
    }
    this_thread::sleep_for(chrono::milliseconds(100));
    // this_thread::yield()        -> this exists lol
  }
}

int main() {
  thread block(blockCount);
  thread wait(ioWait);

  block.join();
  wait.join();

  cout << "\033[2K\rDone!";
}
