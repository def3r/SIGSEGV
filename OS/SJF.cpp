#include <cmath>
#include <cstddef>
#include <iostream>
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

class Processor {
 public:
  Processor() {}
  void init(Processes& procs) {
    freshProcs = procs;
    totalProc = procs.size();
  }
  void compute() {
    while (completeProcs.size() != totalProc) {
      int index = 0;
      for (const auto& proc : freshProcs) {
        if (proc.arrivalTime == tick) {
          readyQueue.push(proc);
          freshProcs.erase(freshProcs.begin() + index);
          continue;
        }
        index++;
      }

      if (!processorOccupied && !readyQueue.empty()) {
        execProc = readyQueue.front();
        std::cout << "Scheduled Process: " << execProc.procName << " @tick "
                  << tick << "\n";
        readyQueue.pop();
        processorOccupied = true;
        burst = &execProc.burstTimeCPU;
        burstType = BurstType::CPU;
        execProcIOBurst = execProc.burstTimeIO;
        tick++;
        continue;
      }

      if (processorOccupied) {
        (*burst)--;
        if (burstType == BurstType::CPU) {
          lastIOBurst++;
        }

        if (*burst <= 0 && burstType == BurstType::CPU) {
          processorOccupied = false;
          std::cout << "Completed Process: " << execProc.procName << " @tick "
                    << tick << "\n";
          completeProcs.push_back(execProc);
        } else if (*burst <= 0 && burstType == BurstType::IO) {
          burst = &execProc.burstTimeCPU;
          burstType = BurstType::CPU;
        }

        if (lastIOBurst >= execProc.burstTimeRate) {
          execProcIOBurst = execProc.burstTimeIO;
          burst = &execProcIOBurst;
          burstType = BurstType::IO;
          lastIOBurst = 0;
        }
      }

      tick++;
    }
  }

 protected:
  enum BurstType { NONE, CPU, IO };

  size_t tick = 0;
  size_t lastIOBurst = 0;
  size_t* burst = nullptr;
  BurstType burstType = BurstType::NONE;
  Process execProc;
  size_t execProcIOBurst = 0;
  bool processorOccupied = false;
  std::queue<Process> readyQueue = {};
  Processes freshProcs = {};
  size_t totalProc = 0;
  Processes completeProcs = {};
};

int main() {
  Processes procs;
  procs.push_back(Process("P0", 0, 24, 2, 5));
  procs.push_back(Process("P1", 3, 17, 3, 6));
  procs.push_back(Process("P2", 8, 50, 2, 5));
  procs.push_back(Process("P3", 15, 10, 3, 6));

  Processor CPU;
  CPU.init(procs);
  CPU.compute();

  return 0;
}
