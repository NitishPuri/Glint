#include "Logger.h"

std::ofstream Logger::log_file_;
std::mutex Logger::mutex_;

