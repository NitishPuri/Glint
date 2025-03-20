#pragma once

#include <vulkan/vulkan.h>

#include <array>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace glint {

struct Vertex {
  glm::vec3 position;
  glm::vec3 color;
  // glm::vec2 texCoord;

  static VkVertexInputBindingDescription getBindingDescription();
  static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
};

class VulkanContext;
class CommandManager;

class Mesh {
 public:
  Mesh(VulkanContext* context, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices = {});
  ~Mesh();

  // Prevent copying
  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

 private:
  // void createVertexBuffer(const std::vector<Vertex>& vertices);
  void createVertexBuffer();
  // void createIndexBuffer(const std::vector<uint32_t>& indices);
  // void createIndexBuffer();
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

 private:
  VulkanContext* m_Context;

  const std::vector<Vertex>& m_Vertices;
  const std::vector<uint32_t>& m_Indices;

  VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
  VkDeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;

  VkBuffer m_IndexBuffer = VK_NULL_HANDLE;
  VkDeviceMemory m_IndexBufferMemory = VK_NULL_HANDLE;

  uint32_t m_IndexCount = 0;
  uint32_t m_VertexCount = 0;
  bool m_HasIndices = false;
};

}  // namespace glint