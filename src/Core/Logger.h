#pragma once

// supress warning due to localtime being unsafe
#pragma warning(disable : 4996)

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#define CONSOLE_LOG 1

class Logger {
 public:
  Logger(const std::string &log_file_path) : log_file_path(log_file_path) {
    log_file.open(log_file_path, std::ios_base::app);  // Open in append mode
    if (!log_file.is_open()) {
      std::cerr << "Unable to open log file: " << log_file_path << std::endl;
    }
  }

  ~Logger() {
    if (log_file.is_open()) {
      log_file.close();
    }
  }

  template <typename... Args>
  void error(Args... args) const {
    std::ostringstream oss;
    oss << get_current_timestamp() << " [ERROR]: ";
    (oss << ... << args);  // Fold expression to handle variadic arguments

    std::string timestamped_message = oss.str();
#if CONSOLE_LOG
    // Log to console
    std::cerr << timestamped_message << std::endl;
#endif

    // Log to file
    if (log_file.is_open()) {
      log_file << timestamped_message << std::endl;
    }
  }

  template <typename... Args>
  void log(Args... args) const {
    std::ostringstream oss;
    oss << get_current_timestamp() << ":[LOG] ";
    (oss << ... << args);  // Fold expression to handle variadic arguments

    std::string timestamped_message = oss.str();
#if CONSOLE_LOG
    // Log to console
    std::clog << timestamped_message << std::endl;
#endif

    // Log to file
    if (log_file.is_open()) {
      log_file << timestamped_message << std::endl;
    }
  }

 private:
  std::string log_file_path;
  mutable std::ofstream log_file;

  std::string get_current_timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%H-%M-%S") << '-' << std::setfill('0') << std::setw(3)
       << ms.count();
    return ss.str();
  }
};
