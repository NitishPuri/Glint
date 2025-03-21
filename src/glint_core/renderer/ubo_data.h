#pragma once

#include <glm/glm.hpp>

namespace glint {

// Uniform buffer object to pass to the shader
struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

}  // namespace glint