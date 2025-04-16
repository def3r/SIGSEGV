#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <iostream>
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

  bool start() {
    std::string cmd = compiler + " " + srcFile.filename().string() + " -o " +
                      execFile.filename().string();
    if (system(cmd.c_str()) != 0) {
      Log::error("Compilation error");
      exit(1);
    }
    Log::success("Compiled " + execFile.filename().string());
    return false;
  }
} Compile;

void childOnComplete(pid_t pid) {
  int status;
  waitpid(pid, &status, 0);
  Log::msg("Child exited with status " + std::to_string(status));
}

int main(int argc, char* argv[]) {
  std::vector<std::string> args(argv + 1, argv + argc);
  if (args.size() == 0) {
    Log::error("No file provided.");
    exit(1);
  }
  std::string file = args.front();

  fs::path srcFile = file;
  if (!fs::exists(srcFile)) {
    Log::error("Cannot find `" + srcFile.filename().string() + "`");
    exit(1);
  }

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
  }
  if (execExists && !fs::exists(execFile)) {
    Log::error("Needs Compiling");
    compile.start();
  }
  close(STDIN_FILENO);

  pid_t pid = fork();
  std::thread([pid]() {
    int status;
    waitpid(pid, &status, 0);
    std::cout << "Child " << pid << " exited with status " << status << "\n";
  }).detach();
  if (pid == 0) {
    execl(execFile.filename().string().c_str(),
          execFile.filename().string().c_str(), nullptr);
    exit(1);
  }

  while (1) {
    if (execExists && !fs::exists(execFile)) {
      Log::msg("srcFile updated");
      Log::msg("killing proc " + std::to_string(pid));
      kill(pid, SIGTERM);
      compile.start();
      pid = fork();
      if (pid == 0) {
        execl(execFile.filename().string().c_str(),
              execFile.filename().string().c_str(), nullptr);
      }
      std::thread([pid]() {
        int status;
        waitpid(pid, &status, 0);
        std::cout << "Child " << pid << " exited with status " << status
                  << "\n";
      }).detach();
      continue;
    }
    // clang-format off
    if (
      fs::last_write_time(srcFile) > fs::last_write_time(execFile)
    ) {  // clang-format on
      Log::msg("srcFile updated");
      Log::msg("killing proc " + std::to_string(pid));
      kill(pid, SIGTERM);
      compile.start();
      pid = fork();
      if (pid == 0) {
        execl(execFile.filename().string().c_str(),
              execFile.filename().string().c_str(), nullptr);
      }
      std::thread([pid]() {
        int status;
        waitpid(pid, &status, 0);
        std::cout << "Child " << pid << " exited with status " << status
                  << "\n";
      }).detach();
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  return 0;
}
