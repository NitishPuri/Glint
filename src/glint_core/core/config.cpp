#include "config.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

#include "logger.h"

namespace glint {

void Config::parseCommandLine(int argc, char** argv) {
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "--enable-logging" || arg == "-l") {
      m_EnableLogging = true;
    } else if (arg == "--disable-logging" || arg == "-L") {
      m_EnableLogging = false;
    } else if (arg == "--enable-validation" || arg == "-v") {
      m_EnableValidationLayers = true;
    } else if (arg == "--disable-validation" || arg == "-V") {
      m_EnableValidationLayers = false;
    }
  }
}

void Config::parseEnvironment() {
  // Check environment variables
  if (const char* loggingEnv = std::getenv("GLINT_ENABLE_LOGGING")) {
    std::string value = loggingEnv;
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    m_EnableLogging = (value == "1" || value == "true" || value == "yes");
  }

  if (const char* validationEnv = std::getenv("GLINT_ENABLE_VALIDATION")) {
    std::string value = validationEnv;
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    m_EnableValidationLayers = (value == "1" || value == "true" || value == "yes");
  }
}

void Config::initialize(int argc, char** argv) {
  instance().parseEnvironment();
  instance().parseCommandLine(argc, argv);
  Logger::enabled(isLoggingEnabled());
}

}  // namespace glint