#include <cmath>
#include <cstddef>
#include <iostream>
#include <ostream>
#include <queue>
#include <string>
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

  void processor() {
    Process execProc;
    size_t lastIOBurst = 0;
    std::cout << "Time (tick)\tDevice\t\tProcess Served\t\tProcessess" << "\n";
    while (totalProc) {
      if (isIdle) {
        std::cout << ticksCPU << "-" << ticksCPU + 1 << "\t\t" << "CPU"
                  << "\t\t" << "-" << "\t\t"
                  << "\n";
      } else {
        std::cout << ticksCPU << "-" << ticksCPU + 1;
      }
      int index = 0;
      for (const auto& proc : procs) {
        if (proc.arrivalTime == ticksCPU) {
          std::cout << "\t\t" << "CPU" << "\t\t" << proc.procName
                    << "[Arrive]\t\t" << "\n";
          readyQ.push(proc);
          procs.erase(procs.begin() + index);
          continue;
        }
        index++;
      }

      if (isIdle && !readyQ.empty()) {
        execProc = readyQ.top();
        readyQ.pop();
        std::cout << "\t\t" << "CPU" << "\t\t" << execProc.procName
                  << "[Sched]:" << execProc.burstTimeCPU << "\t\t" << "\n";
        lastIOBurst = 0;
        isIdle = false;
        // ticksCPU++;
        // ioDevice();
        // std::cout << "\n";
      }

      if (!isIdle) {
        if (--execProc.burstTimeCPU <= 0) {
          std::cout << "\t\t" << "CPU" << "\t\t" << execProc.procName
                    << "[Comp]\t\t" << "\n";
          isIdle = true;
          totalProc--;
          execProc = {};
        } else if (++lastIOBurst >= execProc.burstTimeRate) {
          std::cout << "\t\t" << "CPU" << "\t\t" << execProc.procName
                    << "[Q IO]:" << execProc.burstTimeCPU << "\t\t" << "\n";
          ioQ.push(execProc);
          isIdle = true;
        } else {
          std::cout << "\t\t" << "CPU"
                    << "\t\t" << execProc.procName << ":"
                    << execProc.burstTimeCPU << "\t\t"
                    << "\n";
        }
      }

      ticksCPU++;
      ioDevice();
      std::cout << "\n";
    }
  }

  void ioDevice() {
    if (isIOIdle && !ioQ.empty()) {
      execProcIO = ioQ.top();
      ioQ.pop();
      std::cout << "\t\t" << "IO" << "\t\t" << execProcIO.procName
                << "[Sched]:" << countIOBurst << "\t\t" << "\n";
      countIOBurst = 0;
      isIOIdle = false;
      ticksIO++;
      return;
    }

    if (!isIOIdle) {
      if (++countIOBurst >= execProcIO.burstTimeIO) {
        std::cout << "\t\t" << "IO" << "\t\t" << execProcIO.procName
                  << "[Comp]:" << countIOBurst << "\t\t" << "\n";
        readyQ.push(execProcIO);
        execProcIO = {};
        isIOIdle = true;
      } else {
        std::cout << "\t\t" << "IO" << "\t\t" << execProcIO.procName << ":"
                  << countIOBurst << "\t\t"
                  << "\n";
      }
    }

    ticksIO++;
  }

 private:
  Processes procs = {};
  size_t totalProc = 0;
  size_t ticksCPU = 0;
  size_t ticksIO = 0;
  bool isIdle = true;

  size_t countIOBurst = 0;
  bool isIOIdle = true;
  Process execProcIO;

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
  d.processor();

  return 0;
}
