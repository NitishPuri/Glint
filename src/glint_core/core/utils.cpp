#include "utils.h"

#ifndef SHADER_BIN_DIR
#define SHADER_BIN_DIR "./build/bin/shaders/"
#endif

namespace glint {

std::string getShaderPath(const std::string& shaderName) { return std::string(SHADER_BIN_DIR) + shaderName + ".spv"; }

}  // namespace glint