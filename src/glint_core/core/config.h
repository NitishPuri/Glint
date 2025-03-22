#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

namespace glint {

class Config {
 public:
  static bool isLoggingEnabled() { return instance().m_EnableLogging; }
  static bool areValidationLayersEnabled() { return instance().m_EnableValidationLayers; }

  // New getters for shader and resource paths
  static std::string getShaderPath() { return instance().m_ShaderPath; }
  static std::string getResourcePath() { return instance().m_ResourcePath; }

  // Combine with relative path
  static std::string getShaderFile(const std::string& filename) {
    return (std::filesystem::path(instance().m_ShaderPath) / filename).string() + ".spv";
  }

  static std::string getResourceFile(const std::string& filename) {
    return (std::filesystem::path(instance().m_ResourcePath) / filename).string();
  }

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
    m_ShaderPath = "./build/bin/shaders";
    m_ResourcePath = "./res";

#ifdef _DEBUG
    m_EnableLogging = true;
    m_EnableValidationLayers = true;
#endif
  }

  bool m_EnableLogging;
  bool m_EnableValidationLayers;
  std::string m_ShaderPath;
  std::string m_ResourcePath;
};

}  // namespace glint