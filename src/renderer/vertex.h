#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "glm/glm.hpp"

namespace glint {

// Flags to specify which vertex attributes are enabled
enum class VertexAttributeFlags {
  Position = 1 << 0,
  Color = 1 << 1,
  TexCoord = 1 << 2,

  // Common Combinations
  POSITION_COLOR = Position | Color,
  POSITION_TEXCOORD = Position | TexCoord,
  POSITION_COLOR_TEXCOORD = Position | Color | TexCoord,
};

inline VertexAttributeFlags operator|(VertexAttributeFlags a, VertexAttributeFlags b) {
  return static_cast<VertexAttributeFlags>(static_cast<int>(a) | static_cast<int>(b));
}

inline bool hasAttribute(VertexAttributeFlags flags, VertexAttributeFlags attr) {
  return (static_cast<int>(flags) & static_cast<int>(attr)) != 0;
}

struct Vertex {
  glm::vec3 position;
  glm::vec3 color;
  glm::vec2 texCoord;

  bool operator==(const Vertex& other) const {
    return position == other.position && color == other.color && texCoord == other.texCoord;
  }

  static VkVertexInputBindingDescription getBindingDescription();

  // static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
  static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(
      VertexAttributeFlags flags = VertexAttributeFlags::POSITION_COLOR_TEXCOORD);
};

}  // namespace glint