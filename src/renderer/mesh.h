#pragma once

#include <vulkan/vulkan.h>

#include <array>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "vertex.h"

namespace glint {

class VkContext;
class CommandManager;

class Mesh {
 public:
  Mesh(VkContext* context, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices = {},
       VertexAttributeFlags flags = VertexAttributeFlags::POSITION_COLOR_TEXCOORD);
  ~Mesh();

  // Prevent copying
  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

  VkBuffer getVertexBuffer() const { return m_VertexBuffer; }

 private:
  // void createVertexBuffer(const std::vector<Vertex>& vertices);
  void createVertexBuffer();
  // void createIndexBuffer(const std::vector<uint32_t>& indices);
  void createIndexBuffer();
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

 private:
  VkContext* m_Context;

  VertexAttributeFlags m_FormatFlags;
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