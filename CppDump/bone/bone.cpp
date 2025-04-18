// FIX:
// C R I T I C A L: NEED AN URGENT REFACTOR

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <ostream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

namespace ANSI {
std::string RED = "\033[31m";
std::string GREEN = "\033[32m";
std::string YELLOW = "\033[33m";
std::string GREY = "\033[90m";
std::string RESET = "\033[0m";
std::string CLEAR = "\033[2J";
std::string GOTOTOP = "\033[H";
}  // namespace ANSI

namespace Log {
void msg(const std::string& msg) {
  std::cout << ANSI::GREY << "[bone]: " << ANSI::RESET << msg << ANSI::RESET
            << "\n";
}
void warning(const std::string& msg) {
  std::cout << ANSI::GREY << "[bone]: " << ANSI::YELLOW << msg << ANSI::RESET
            << "\n";
}
void error(const std::string& msg) {
  std::cout << ANSI::GREY << "[bone]: " << ANSI::RED << msg << ANSI::RESET
            << "\n";
  exit(1);
}
void success(const std::string& msg) {
  std::cout << ANSI::GREY << "[bone]: " << ANSI::GREEN << msg << ANSI::RESET
            << "\n";
}
}  // namespace Log

typedef struct Compile {
  std::string compiler = "";
  fs::path srcFile = "";
  fs::path execFile = "";

  bool start() const {
    std::string cmd =
        compiler + " " + srcFile.string() + " -o " + execFile.string();
    if (system(cmd.c_str()) != 0) {
      Log::error("Compilation error");
    }
    Log::success("Compiled " + execFile.filename().string());
    return false;
  }
} Compile;

typedef struct Child {
  pid_t pid;
  std::string execFile = "";
  struct termios resetAttr, boneAttr;

  inline static std::mutex mtx;
  inline static std::condition_variable cv;
  inline static std::atomic<bool> isExecuting = false;

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

    // std::thread([this]() {
    //   int status;
    //   waitpid(pid, &status, 0);
    //   getTTY();
    //   Log::msg("Child " + std::to_string(pid) + " exited with status " +
    //            std::to_string(status));
    // }).detach();

    return true;
  }

  bool despawn() {
    // std::unique_lock<std::mutex> lock(mtx);
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

void cleanup() {
  tcsetattr(STDIN_FILENO, TCSANOW, &oldAttr);
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
  tcsetpgrp(STDIN_FILENO, getpgrp());
}

int main(int argc, char* argv[]) {
  std::vector<std::string> args(argv + 1, argv + argc);

  if (args.size() == 0) {
    Log::error("No file provided.");
  }
  std::string file = args.front();

  fs::path srcFile = file;
  if (!fs::exists(srcFile)) {
    Log::error("Cannot find `" + srcFile.filename().string() + "`");
  }

  tcgetattr(STDIN_FILENO, &oldAttr);
  newAttr = oldAttr;
  newAttr.c_lflag &= ~(ICANON | ECHO);

  atexit(cleanup);

  Compile compile = {.srcFile = srcFile};
  enum Extension { NONE, C, CPP, OHTER } extension = Extension::NONE;
  if (srcFile.has_extension()) {
    if (srcFile.extension() == ".c" || srcFile.extension() == ".C") {
      compile.compiler = "gcc";
      extension = Extension::C;
      Log::msg("Extension: C");
    } else if (srcFile.extension() == ".cpp" || srcFile.extension() == ".CPP") {
      compile.compiler = "g++";
      extension = Extension::CPP;
      Log::msg("Extension: Cpp");
    } else {
      extension = Extension::OHTER;
      Log::msg("Extension: Other");
    }
  }

  bool execExists = false;
  fs::path execFile = srcFile;
  if (extension >= Extension::C || extension <= Extension::CPP) {
    execExists = true;
    execFile.replace_extension(".out");
    compile.execFile = execFile;
  } else if (extension == Extension::OHTER) {
    Log::error("Unknown Extension: " + srcFile.extension().string());
  }
  if (execExists && !fs::exists(execFile)) {
    Log::warning("No executable found");
    compile.start();
  }

  Child child(oldAttr, execFile.string());
  child.spawn();

  std::thread([&] {
    while (1) {
      if (execExists && !fs::exists(execFile)) {
        child.respawn(compile);
        continue;
      }

      if (fs::last_write_time(srcFile) > fs::last_write_time(execFile)) {
        child.respawn(compile);
      }

      // TODO:
      // Improve this, as this just wastes cpu cycles in the bg rn
      std::this_thread::sleep_for(std::chrono::seconds(1));
      // Log::msg("Check" + std::to_string((fs::last_write_time(srcFile) >
      //                                    fs::last_write_time(execFile))));
    }
  }).detach();

  std::atomic<bool> keepInteractive = false;
  int pid = 0;
  while (1) {
    int status;
    pid = child.pid;
    waitpid(pid, &status, 0);
    child.getTTY();
    Log::msg("Child " + std::to_string(child.pid) + " exited with status " +
             std::to_string(status));
    if (Child::isExecuting) {
      if (pid != child.pid) {
        continue;
      }
      Child::isExecuting = false;
    }

    // std::thread interactive([&child, &oldAttr, &keepInteractive] {
    char c;
    while (!Child::isExecuting && tcgetpgrp(STDIN_FILENO) == getpgrp()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(150));
      if (tcgetpgrp(STDIN_FILENO) != getpgrp()) {
        break;
      }
      if (read(STDIN_FILENO, &c, 1) != 1) {
        continue;
      }

      switch (c) {
        case 'q':
        case 'Q': {
          tcsetattr(STDIN_FILENO, TCSANOW, &oldAttr);
          exit(0);
        }
        case 'r' | 'R':
        case 'R': {
          child.spawn();
          break;
        }
        case 'c' | 'C': {
          std::cout << ANSI::CLEAR << ANSI::GOTOTOP << std::flush;
        }
        default:
          break;
      }
    }
    // });
    // interactive.detach();

    // {
    //   std::unique_lock<std::mutex> lock(child.mtx);
    //   child.cv.wait(lock, [&] { return Child::isExecuting; });
    // }
    // keepInteractive = false;
    // write(STDIN_FILENO, "\n", 1);
  }

  return 0;
}
