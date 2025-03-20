#include <cmath>
#include <cstddef>
#include <iostream>
#include <mutex>
#include <ostream>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <vector>

typedef struct Process {
  std::string procName;
  size_t arrivalTime;
  size_t burstTimeCPU;
  size_t burstTimeIO;
  size_t burstTimeRate;  // IO burst after every n CPU bursts

  Process() {}
  Process(std::string&& name,
          size_t at,
          size_t btCPU,
          size_t btIO,
          size_t btr) {
    procName = std::move(name);
    arrivalTime = at;
    burstTimeCPU = btCPU;
    burstTimeIO = btIO;
    burstTimeRate = btr;
  }
} Process;
typedef std::vector<Process> Processes;

class Device {
 public:
  size_t MAX_TICKS = 500;
  Device() : readyQ(cmp), ioQ(cmp) {}
  void init(Processes& procs) {
    this->procs = procs;
    totalProc = procs.size();
  }

  void start() {
    working = true;
    std::thread cpu(&Device::processor, this);
    std::thread io(&Device::ioDevice, this);
    cpu.join();
    io.join();
  }

  void processor() {
    Process execProc;
    size_t lastIOBurst = 0;
    while (ticksCPU <= 500) {
      int index = 0;
      for (const auto& proc : procs) {
        if (proc.arrivalTime == ticksCPU) {
          std::cout << "Process Arrived:\t\t" << proc.procName << "\t\t@tick "
                    << ticksCPU << "\n";
          mtx.lock();
          readyQ.push(proc);
          mtx.unlock();
          procs.erase(procs.begin() + index);
          continue;
        }
        index++;
      }

      if (isIdle && !readyQ.empty()) {
        mtx.lock();
        execProc = readyQ.top();
        readyQ.pop();
        mtx.unlock();
        std::cout << "Process Scheduled:\t\t" << execProc.procName
                  << "\t\t@tick " << ticksCPU << "\n";
        lastIOBurst = 0;
        isIdle = false;
        ticksCPU++;
        continue;
      }

      if (!isIdle) {
        if (--execProc.burstTimeCPU <= 0) {
          std::cout << "Process [Completed]:\t\t" << execProc.procName
                    << "\t\t@tick " << ticksCPU << "\n";
          isIdle = true;
        } else if (++lastIOBurst >= execProc.burstTimeRate) {
          std::cout << "Process Queued for IO:\t\t" << execProc.procName
                    << "\t\t@tick " << ticksCPU << "\n";
          mtx.lock();
          ioQ.push(execProc);
          mtx.unlock();
          isIdle = true;
        }
      }

      ticksCPU++;
      std::this_thread::yield();
    }
    mtx.lock();
    working = false;
    mtx.unlock();
  }

  void ioDevice() {
    std::cout << "Started\n";
    Process execProc;
    size_t ticksIO = 0;
    size_t countIOBurst = 0;
    bool isIdle = true;
    while (working) {
      if (isIdle && !ioQ.empty()) {
        mtx.lock();
        execProc = ioQ.top();
        ioQ.pop();
        mtx.unlock();
        std::cout << "[IO] Scheduled:\t\t" << execProc.procName
                  << "\t\t@tick-io " << ticksIO << "\n"
                  << std::flush;
        countIOBurst = 0;
        isIdle = false;
        ticksIO++;
        continue;
      }

      if (!isIdle) {
        if (++countIOBurst >= execProc.burstTimeIO) {
          std::cout << "[IO] Completed:\t\t" << execProc.procName
                    << "\t\t@tick-io " << ticksIO << "\n"
                    << std::flush;
          mtx.lock();
          readyQ.push(execProc);
          mtx.unlock();
          isIdle = true;
        }
      }

      ticksIO++;
      std::this_thread::yield();
    }
    std::cout << "Ended\n";
  }

 private:
  Processes procs = {};
  size_t totalProc = 0;
  size_t ticksCPU = 0;
  size_t ticksIO = 0;
  bool isIdle = true;
  bool working = false;
  std::mutex mtx;

  inline static auto cmp = [](const Process& left, const Process& right) {
    if (left.burstTimeCPU != right.burstTimeCPU)
      return left.burstTimeCPU > right.burstTimeCPU;
    return left.arrivalTime > right.arrivalTime;
  };
  std::priority_queue<Process, Processes, decltype(cmp)> readyQ;
  std::priority_queue<Process, Processes, decltype(cmp)> ioQ;
};

int main() {
  Processes procs;
  procs.push_back(Process("P0", 0, 24, 2, 5));
  procs.push_back(Process("P1", 3, 17, 3, 6));
  procs.push_back(Process("P2", 8, 50, 2, 5));
  procs.push_back(Process("P3", 15, 10, 3, 6));

  Device d;
  d.init(procs);
  d.start();

  return 0;
}
