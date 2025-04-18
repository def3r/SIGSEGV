#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

// clang-format off
namespace ANSI {
std::string CLEAR   = "\033[2J";
std::string GOTOTOP = "\033[H";

std::string RED     = "\033[31m";
std::string GREY    = "\033[90m";
std::string RESET   = "\033[0m";
std::string GREEN   = "\033[32m";
std::string YELLOW  = "\033[33m";
}  // namespace ANSI

namespace Log {
enum class Level { Info, Warning, Error, Success };

#define HELP(key, help)                                                     \
  std::cout << bone() << ANSI::GREY << key << "\t\t" << ANSI::RESET << help \
            << "\n"

inline std::string bone() { return std::string(ANSI::GREY + "[bone]: " + ANSI::RESET);}
inline void help() {
  std::cout << ANSI::YELLOW << "[bone h]" << "\n";
  HELP("c", "Clear Screen");
  HELP("r", "Restart Program");
  HELP("q", "Quit bone");
  HELP("h", "H1l[]!");
}

void log(Level level, const std::string& msg) {
  std::string color = "";
  switch (level) {
    case Level::Info:    color = ANSI::RESET; break;
    case Level::Warning: color = ANSI::YELLOW; break;
    case Level::Error:   color = ANSI::RED; break;
    case Level::Success: color = ANSI::GREEN; break;
  }
  std::cout << bone() << color << msg << ANSI::RESET << "\n";
  if (level == Level::Error) {
    exit(1);
  }
}

inline void msg(const std::string& msg) { log(Level::Info, msg); }
inline void error(const std::string& msg) { log(Level::Error, msg); }
inline void warning(const std::string& msg) { log(Level::Warning, msg); }
inline void success(const std::string& msg) { log(Level::Success, msg); }
inline void clear() { std::cout << ANSI::CLEAR << ANSI::GOTOTOP << std::flush; }
}  // namespace Log
// clang-format on

typedef struct Compile {
  fs::path srcFile = "";
  fs::path execFile = "";
  bool execExists = false;
  std::string compiler = "";
  enum Extension { NONE, C, CPP, OHTER } extension = Extension::NONE;

  Compile(const std::string& inputFile) : srcFile(inputFile) {}

  bool start() const {
    std::string cmd =
        compiler + " " + srcFile.string() + " -o " + execFile.string();
    if (system(cmd.c_str()) != 0) {
      Log::error("Compilation error");
    }
    Log::success("Compiled " + execFile.filename().string());
    return false;
  }

  void verifyExecFile() {
    execFile = srcFile;
    if (extension >= Extension::C || extension <= Extension::CPP) {
      execExists = true;
      execFile.replace_extension(".out");
    } else if (extension == Extension::OHTER) {
      Log::error("Unknown Extension: " + srcFile.extension().string());
    }
    if (execExists && !fs::exists(execFile)) {
      Log::warning("No executable found");
      start();
    }
  }

  void verifySrcFile() {
    if (!fs::exists(srcFile)) {
      Log::error("Cannot find `" + srcFile.filename().string() + "`");
    }
    if (srcFile.has_extension()) {
      if (srcFile.extension() == ".c" || srcFile.extension() == ".C") {
        compiler = "gcc";
        extension = Extension::C;
        Log::msg("Extension: C");
      } else if (srcFile.extension() == ".cpp" ||
                 srcFile.extension() == ".CPP") {
        compiler = "g++";
        extension = Extension::CPP;
        Log::msg("Extension: Cpp");
      } else {
        extension = Extension::OHTER;
        Log::msg("Extension: Other");
      }
    }
  }
} Compile;

typedef struct Child {
  pid_t pid;
  std::string execFile = "";
  struct termios resetAttr, boneAttr;
  std::atomic<bool> isExecuting = false;

  Child(const struct termios& resetAttr, const std::string& execFile)
      : resetAttr(resetAttr), execFile(execFile) {
    boneAttr = resetAttr;
    boneAttr.c_lflag &= ~(ICANON | ECHO);
  }

  bool spawn() {
    if (isExecuting) {
      return false;
    }
    isExecuting = true;

    pid = fork();
    if (pid < 0) {
      return false;
    }
    if (pid == 0) {
      giveTTY(0, getpid());
      execl(execFile.c_str(), execFile.c_str(), nullptr);
      exit(1);
    }

    giveTTY(pid, pid);

    return true;
  }

  bool despawn() {
    kill(pid, SIGTERM);
    isExecuting = false;
    std::this_thread::yield();
    return true;
  }

  bool respawn(const Compile& compile) {
    getTTY();
    Log::msg("srcFile updated");
    Log::msg("killing proc " + std::to_string(pid));
    despawn();
    compile.start();
    return spawn();
  }
  bool respawn() {
    getTTY();
    Log::msg("restarting proc " + std::to_string(pid));
    despawn();
    return spawn();
  }

  void giveTTY(pid_t grp, pid_t pid) {
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
    tcsetattr(STDIN_FILENO, TCSANOW, &resetAttr);
    setpgid(grp, grp);
    tcsetpgrp(STDIN_FILENO, pid);
  }

  void getTTY() {
    signal(SIGTTOU, SIG_IGN);
    tcsetpgrp(STDIN_FILENO, getpgrp());
    tcflush(STDIN_FILENO, TCIFLUSH);
    signal(SIGTTOU, SIG_DFL);
    tcsetattr(STDIN_FILENO, TCSANOW, &boneAttr);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
  }
} Child;

struct termios newAttr, oldAttr;

void getTermAttr() {
  tcgetattr(STDIN_FILENO, &oldAttr);
  newAttr = oldAttr;
  newAttr.c_lflag &= ~(ICANON | ECHO);
}

void cleanup() {
  tcsetattr(STDIN_FILENO, TCSANOW, &oldAttr);
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
  tcsetpgrp(STDIN_FILENO, getpgrp());
}

void dispatchMonitor(const Compile& compile, Child& child) {
  std::thread([&] {
    while (1) {
      if (compile.execExists && !fs::exists(compile.execFile)) {
        child.respawn(compile);
        continue;
      }

      if (fs::last_write_time(compile.srcFile) >
          fs::last_write_time(compile.execFile)) {
        child.respawn(compile);
      }

      // TODO:
      // Improve this, as this just wastes cpu cycles in the bg rn
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }).detach();
}

void interactiveMode(Child& child) {
  char c;
  while (!child.isExecuting && tcgetpgrp(STDIN_FILENO) == getpgrp()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    if (tcgetpgrp(STDIN_FILENO) != getpgrp()) {
      break;
    }
    if (read(STDIN_FILENO, &c, 1) != 1) {
      continue;
    }

    // clang-format off
    switch (c) {
      case 'h': case 'H': { Log::help(); break; }
      case 'c': case 'C': { Log::clear(); break; }
      case 'r': case 'R': { child.spawn(); break; }
      case 'q': case 'Q': {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldAttr);
        exit(0);
      }
      default: break;
    }
    // clang-format on
  }
}

void boneManager(const Compile& compile, Child& child) {
  dispatchMonitor(compile, child);

  int pid = 0;
  while (1) {
    int status;
    pid = child.pid;
    waitpid(pid, &status, 0);
    child.getTTY();
    Log::msg("Proc " + std::to_string(child.pid) + " exited with status " +
             std::to_string(status));
    if (child.isExecuting) {
      if (pid != child.pid) {
        continue;
      }
      child.isExecuting = false;
    }

    interactiveMode(child);
  }
}

int main(int argc, char* argv[]) {
  std::vector<std::string> args(argv + 1, argv + argc);
  if (args.size() == 0) {
    Log::error("No file provided.");
  }
  std::string file = args.front();

  getTermAttr();
  atexit(cleanup);

  Compile compile(file);
  compile.verifySrcFile();
  compile.verifyExecFile();

  Child child(oldAttr, compile.execFile.string());
  child.spawn();

  boneManager(compile, child);
}
