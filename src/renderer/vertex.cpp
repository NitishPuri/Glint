#include "vertex.h"

#include "logger.h"

namespace glint {

VkVertexInputBindingDescription Vertex::getBindingDescription() {
  LOGFN;
  VkVertexInputBindingDescription bindingDescription{};
  bindingDescription.binding = 0;
  bindingDescription.stride = sizeof(Vertex);
  bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions(VertexAttributeFlags flags) {
  LOGFN;
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

  VkVertexInputAttributeDescription positionAttributeDescription{};
  positionAttributeDescription.binding = 0;
  positionAttributeDescription.location = 0;
  positionAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
  positionAttributeDescription.offset = offsetof(Vertex, position);
  attributeDescriptions.push_back(positionAttributeDescription);

  if (hasAttribute(flags, VertexAttributeFlags::Color)) {
    VkVertexInputAttributeDescription colorAttributeDescription{};
    colorAttributeDescription.binding = 0;
    colorAttributeDescription.location = 1;
    colorAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
    colorAttributeDescription.offset = offsetof(Vertex, color);
    attributeDescriptions.push_back(colorAttributeDescription);
  }

  if (hasAttribute(flags, VertexAttributeFlags::TexCoord)) {
    VkVertexInputAttributeDescription texCoordAttributeDescription{};
    texCoordAttributeDescription.binding = 0;
    texCoordAttributeDescription.location = 2;
    texCoordAttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
    texCoordAttributeDescription.offset = offsetof(Vertex, texCoord);
    attributeDescriptions.push_back(texCoordAttributeDescription);
  }

  return attributeDescriptions;
}
}  // namespace glint