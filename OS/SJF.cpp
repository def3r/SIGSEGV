#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
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
  size_t startTime = SIZE_MAX;
  size_t completionTime;
  size_t burstRemainCPU;

  Process() {}
  Process(std::string&& name,
          size_t at,
          size_t btCPU,
          size_t btIO,
          size_t btr) {
    procName = std::move(name);
    arrivalTime = at;
    burstTimeCPU = btCPU;
    burstRemainCPU = btCPU;
    burstTimeIO = btIO;
    burstTimeRate = btr;
  }
  size_t turnAroundTime() { return completionTime - arrivalTime; }
  size_t waitingTime() { return turnAroundTime() - burstTimeCPU; }
  size_t responseTime() { return startTime - arrivalTime; }
} Process;
typedef std::vector<Process> Processes;

class Device {
 public:
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
        std::cout << ticksCPU /* << "-" << ticksCPU + 1*/ << "\t\t" << "CPU"
                  << "\t\t" << "-" << "\t\t"
                  << "\n";
      } else {
        std::cout << ticksCPU;
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
                  << "[Sched]:" << execProc.burstRemainCPU << "\t\t" << "\n";
        execProc.startTime = std::min(execProc.startTime, ticksCPU);
        lastIOBurst = 0;
        isIdle = false;
        ioDevice();
        ticksCPU++;
        std::cout << "\n";
        continue;
      }

      if (!isIdle) {
        if (--execProc.burstRemainCPU <= 0) {
          std::cout << "\t\t" << "CPU" << "\t\t" << execProc.procName
                    << "[Comp]\t\t" << "\n";
          isIdle = true;
          totalProc--;
          execProc.completionTime = ticksCPU;
          completedProcs.push_back(execProc);
          execProc = {};
        } else if (++lastIOBurst >= execProc.burstTimeRate) {
          std::cout << "\t\t" << "CPU" << "\t\t" << execProc.procName
                    << "[Q IO]:" << execProc.burstRemainCPU << "\t\t" << "\n";
          ioQ.push(execProc);
          isIdle = true;
        } else {
          std::cout << "\t\t" << "CPU"
                    << "\t\t" << execProc.procName << ":"
                    << execProc.burstRemainCPU << "\t\t"
                    << "\n";
        }
      }

      ioDevice();
      ticksCPU++;
      std::cout << "\n";
    }
    Debug();
  }

  void Debug() {
    for (auto& proc : completedProcs) {
      std::cout << proc.procName << ":\n\t\tStart time:\t\t" << proc.startTime
                << "\n\t\tResponse Time:\t\t" << proc.responseTime()
                << "\n\t\tCompletion time:\t" << proc.completionTime
                << "\n\t\tTurnaround Time:\t" << proc.turnAroundTime()
                << "\n\t\tWaiting Time:\t\t" << proc.waitingTime() << "\n";
    }
    std::cout << "Avg Waiting Time: " << avgWaitingTime();
  }

  double avgWaitingTime() {
    double sum = 0;
    for (auto& proc : completedProcs) {
      sum += proc.waitingTime();
    }
    return (double)(sum / completedProcs.size());
  }

  void ioDevice() {
    if (isIOIdle && !ioQ.empty()) {
      execProcIO = ioQ.top();
      ioQ.pop();
      std::cout << "\t\t" << "IO" << "\t\t" << execProcIO.procName
                << "[Sched]:" << countIOBurst << "\t\t" << "\n";
      countIOBurst = 0;
      isIOIdle = false;
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
  }

 private:
  Processes completedProcs = {};
  Processes procs = {};
  size_t totalProc = 0;
  size_t ticksCPU = 0;
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
