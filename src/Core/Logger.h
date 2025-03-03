#pragma once

// supress warning due to localtime being unsafe
#pragma warning(disable : 4996)

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

#define CONSOLE_LOG 1

class Logger {
 public:
  static void init(const std::string& log_file_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (log_file_.is_open()) {
      error("Logger already initialized");
      return;
    }
    log_file_.open(log_file_path, std::ios_base::app);  // Open in append mode
    if (!log_file_.is_open()) {
      std::cerr << "Unable to open log file: " << log_file_path << std::endl;
    }
  }

  static void shutdown() {
    // Close the log file
    if (log_file_.is_open()) {
      log_file_.close();
    }
  }

  template <typename... Args>
  static void error(Args... args) {
    log_message("ERROR", std::cerr, args...);
  }

  template <typename... Args>
  static void log(Args... args) {
    log_message("LOG", std::clog, args...);
  }

 private:
  //   static std::string log_file_path_;
  static std::ofstream log_file_;
  static std::mutex mutex_;

  static std::string get_current_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%H-%M-%S") << '-' << std::setfill('0') << std::setw(3)
       << ms.count();
    return ss.str();
  }

  template <typename... Args>
  static void log_message(const std::string& level, std::ostream& console_stream, Args... args) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ostringstream oss;
    oss << get_current_timestamp() << " [" << level << "]: ";
    (oss << ... << args);  // Fold expression to handle variadic arguments

    std::string timestamped_message = oss.str();
#if CONSOLE_LOG
    // Log to console
    console_stream << timestamped_message << std::endl;
#endif

    // Log to file
    if (log_file_.is_open()) {
      log_file_ << timestamped_message << std::endl;
    }
  }
};

std::ofstream Logger::log_file_;
std::mutex Logger::mutex_;