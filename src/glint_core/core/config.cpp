#include "config.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

#include "logger.h"

#ifndef BASE_DIR
#define BASE_DIR "."
#endif

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
    } else if (arg == "--shader-path" || arg == "-s") {
      if (i + 1 < argc) {
        m_ShaderPath = argv[++i];
      }
    } else if (arg == "--resource-path" || arg == "-r") {
      if (i + 1 < argc) {
        m_ResourcePath = argv[++i];
      }
    } else if (arg.substr(0, 2) == "--") {
      // Custom options
      std::string option = arg.substr(2);
      std::string value = "";
      if (i + 1 < argc) {
        value = argv[++i];
      }
      m_cliOptions[option] = value;
    } else {
      m_cliOptions[arg] = "true";
      // std::cerr << "Unknown option: " << arg << std::endl;
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

  if (const char* shaderPathEnv = std::getenv("GLINT_SHADER_PATH")) {
    m_ShaderPath = shaderPathEnv;
  }

  if (const char* resourcePathEnv = std::getenv("GLINT_RESOURCE_PATH")) {
    m_ResourcePath = resourcePathEnv;
  }
}

void Config::initialize(int argc, char** argv) {
  // Default settings
  instance().m_ShaderPath = std::string(BASE_DIR) + "/build/bin/shaders";
  instance().m_ResourcePath = std::string(BASE_DIR) + "/res";

  instance().parseEnvironment();
  instance().parseCommandLine(argc, argv);

  Logger::enabled(isLoggingEnabled());

  if (isLoggingEnabled()) {
    std::cout << "Configuration:" << std::endl;
    std::cout << "  Logging: " << (isLoggingEnabled() ? "enabled" : "disabled") << std::endl;
    std::cout << "  Validation Layers: " << (areValidationLayersEnabled() ? "enabled" : "disabled") << std::endl;
    std::cout << "  Shader Path: " << getShaderPath() << std::endl;
    std::cout << "  Resource Path: " << getResourcePath() << std::endl;
    if (!instance().m_cliOptions.empty()) {
      std::cout << "  Custom Options:" << std::endl;
      for (const auto& [option, value] : instance().m_cliOptions) {
        std::cout << "  " << option << ": " << value << std::endl;
      }
    }
  }
}

}  // namespace glint