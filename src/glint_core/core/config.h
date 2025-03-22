#pragma once

#include <string>
#include <unordered_map>

namespace glint {

class Config {
 public:
  static bool isLoggingEnabled() { return instance().m_EnableLogging; }
  static bool areValidationLayersEnabled() { return instance().m_EnableValidationLayers; }

  static void initialize(int argc, char** argv);

 private:
  static Config& instance() {
    static Config config;
    return config;
  }

  void parseCommandLine(int argc, char** argv);
  void parseEnvironment();

 private:
  Config() {
    // Default settings
    m_EnableLogging = false;
    m_EnableValidationLayers = false;

#ifdef _DEBUG
    m_EnableLogging = true;
    m_EnableValidationLayers = true;
#endif
  }

  bool m_EnableLogging;
  bool m_EnableValidationLayers;
};

}  // namespace glint