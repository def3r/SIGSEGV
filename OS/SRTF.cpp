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

#define LOG_TICK(ticks) std::cout << ticks;
#define LOG(tick, device, procData) \
  std::cout << tick << "\t" << device << "\t\t" << procData << "\n";
#define LOG_DEBUG(name, label, info) \
  std::cout << name << "\n\t\t" << label "\t" << info;

typedef struct Process {
  enum State { READY, RUNNING, BLOCKED, TERMINATED };
  std::string procName;
  size_t arrivalTime = SIZE_MAX;
  size_t burstTimeCPU = SIZE_MAX;
  size_t burstTimeIO = SIZE_MAX;
  size_t burstTimeRate = SIZE_MAX;  // IO burst after every n CPU bursts
  size_t startTime = SIZE_MAX;
  size_t completionTime;
  size_t burstRemainCPU = SIZE_MAX;
  size_t lastIOBurst = 0;
  State state;

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
  State exec() {
    state = State::RUNNING;
    if (--burstRemainCPU <= 0) {
      state = State::TERMINATED;
    } else if (++lastIOBurst >= burstTimeRate) {
      refreshIOBurst();
      state = State::BLOCKED;
    }
    return state;
  }
  void refreshIOBurst() { lastIOBurst = 0; }
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

  void start() { processor(); }

  void processor() {
    Process execProc;
    LOG("Time (tick)", "Device", "Process Served")
    while (totalProc) {
      LOG_TICK(ticksCPU)
      if (isCPUIdle) {
        LOG("\t", "CPU", "-");
      }
      FreshArrivals();

      if (!isCPUIdle) {
        execProc.exec();
        if (execProc.state == Process::State::TERMINATED) {
          LOG("\t", "CPU", execProc.procName << "[Comp]");
          isCPUIdle = true;
          totalProc--;
          execProc.completionTime = ticksCPU;
          completedProcs.push_back(execProc);
          execProc = {};
        } else if (execProc.state == Process::State::BLOCKED) {
          LOG("\t", "CPU",
              execProc.procName << "[Q IO]:" << execProc.burstRemainCPU);
          ioQ.push(execProc);
          isCPUIdle = true;
          execProc = {};
        } else {
          LOG("\t", "CPU", execProc.procName << ":" << execProc.burstRemainCPU)
        }
      }

      ioDevice();

      bool toSchedule =
          !readyQ.empty() &&
          (isCPUIdle || readyQ.top().burstRemainCPU < execProc.burstRemainCPU);
      if (toSchedule) {
        auto proc = readyQ.top();
        LOG("\t", "CPU", proc.procName << "[Sched]")
        readyQ.pop();
        if (!isCPUIdle) {
          readyQ.push(execProc);
        }
        execProc = proc;
        execProc.startTime = std::min(execProc.startTime, ticksCPU);
        isCPUIdle = false;
      }

      ticksCPU++;
      LOG("", "", "")
    }
  }

  void ioDevice() {
    if (!isIOIdle) {
      if (++countIOBurst >= execProcIO.burstTimeIO) {
        LOG("\t", "IO", execProcIO.procName << "[Comp]:" << countIOBurst)
        readyQ.push(execProcIO);
        execProcIO = {};
        isIOIdle = true;
      } else {
        LOG("\t", "IO", execProcIO.procName << ":" << countIOBurst)
      }
    }

    if (isIOIdle && !ioQ.empty()) {
      execProcIO = ioQ.top();
      ioQ.pop();
      countIOBurst = 0;
      isIOIdle = false;
      LOG("\t", "IO", execProcIO.procName << "[Sched]:" << countIOBurst)
    }
  }

  void debug() {
    for (auto& proc : completedProcs) {
      LOG_DEBUG(proc.procName, "Arrival Time:\t", proc.arrivalTime)
      LOG_DEBUG("", "Start Time:\t", proc.startTime)
      LOG_DEBUG("", "Response Time:\t", proc.responseTime())
      LOG_DEBUG("", "Completion Time:", proc.completionTime)
      LOG_DEBUG("", "Turnaround Time:", proc.turnAroundTime())
      LOG_DEBUG("", "Waiting Time:\t", proc.waitingTime() << "\n")
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

 private:
  Processes completedProcs = {};
  Processes procs = {};
  size_t totalProc = 0;
  size_t ticksCPU = 0;
  bool isCPUIdle = true;

  size_t countIOBurst = 0;
  bool isIOIdle = true;
  Process execProcIO;

  inline static auto cmp = [](const Process& left, const Process& right) {
    if (left.burstRemainCPU != right.burstRemainCPU)
      return left.burstRemainCPU > right.burstRemainCPU;
    return left.arrivalTime > right.arrivalTime;
  };
  std::priority_queue<Process, Processes, decltype(cmp)> readyQ;
  std::priority_queue<Process, Processes, decltype(cmp)> ioQ;

  void FreshArrivals() {
    int index = 0;
    for (auto& proc : procs) {
      if (proc.arrivalTime == ticksCPU) {
        LOG("\t", "CPU", proc.procName << "[Arrive]")
        proc.state = Process::State::READY;
        readyQ.push(proc);
        procs.erase(procs.begin() + index);
        continue;
      }
      index++;
    }
  }
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
  d.debug();

  return 0;
}
