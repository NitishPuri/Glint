#include "mesh.h"

#include <unordered_map>

#include "logger.h"
#include "vk_context.h"
#include "vk_utils.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace std {
template <>
struct hash<glint::Vertex> {
  size_t operator()(glint::Vertex const& vertex) const {
    return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
           (hash<glm::vec2>()(vertex.texCoord) << 1);
  }
};
}  // namespace std

namespace glint {

// TODO: Seperate the model loading and vulkan buffer creation
std::unique_ptr<Mesh> loadModel(VkContext* context, const std::string modelPath) {
  LOGFN;

  LOG("Load Model");
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
    throw std::runtime_error(warn + err);
  }

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  LOG("Vertex Count in model: ", attrib.vertices.size() / 3);
  std::unordered_map<Vertex, uint32_t> uniqueVertices{};

  for (const auto& shape : shapes) {
    for (const auto& index : shape.mesh.indices) {
      Vertex vertex{};
      vertex.position = {attrib.vertices[3 * index.vertex_index + 0], attrib.vertices[3 * index.vertex_index + 1],
                         attrib.vertices[3 * index.vertex_index + 2]};

      vertex.texCoord = {attrib.texcoords[2 * index.texcoord_index + 0],
                         1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

      vertex.color = {1.0f, 1.0f, 1.0f};

      if (uniqueVertices.count(vertex) == 0) {
        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
        vertices.push_back(vertex);
      }

      indices.push_back(uniqueVertices[vertex]);
    }
  }

  LOG("Unique Vertex Count: ", vertices.size());

  return std::make_unique<Mesh>(context, vertices, indices, VertexAttributeFlags::POSITION_COLOR_TEXCOORD);
}

Mesh::Mesh(VkContext* context, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
           VertexAttributeFlags flags)
    : m_Context(context),
      m_Vertices(vertices),
      m_Indices(indices),
      m_IndexCount(static_cast<uint32_t>(indices.size())),
      m_VertexCount(static_cast<uint32_t>(vertices.size())),
      m_HasIndices(!indices.empty()),
      m_FormatFlags(flags) {
  LOGFN;
  createVertexBuffer();
  if (m_HasIndices) {
    createIndexBuffer();
  }

  LOG("Mesh created with", m_Vertices.size(), "vertices and", m_Indices.size(), "indices");
}

Mesh::~Mesh() {
  LOGFN;
  VkDevice device = m_Context->getDevice();

  LOGCALL(vkDestroyBuffer(device, m_VertexBuffer, nullptr));
  LOGCALL(vkFreeMemory(device, m_VertexBufferMemory, nullptr));

  if (m_HasIndices) {
    LOGCALL(vkDestroyBuffer(device, m_IndexBuffer, nullptr));
    LOGCALL(vkFreeMemory(device, m_IndexBufferMemory, nullptr));
  }
}

std::unique_ptr<Mesh> Mesh::loadModel(VkContext* context, const std::string modelPath) {
  return glint::loadModel(context, modelPath);
}

void Mesh::createVertexBuffer() {
  LOGFN;
  VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  VkUtils::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                        stagingBufferMemory);

  void* data;
  vkMapMemory(m_Context->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, m_Vertices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(m_Context->getDevice(), stagingBufferMemory);

  VkUtils::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory);

  copyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

  vkDestroyBuffer(m_Context->getDevice(), stagingBuffer, nullptr);
  vkFreeMemory(m_Context->getDevice(), stagingBufferMemory, nullptr);
}

void Mesh::createIndexBuffer() {
  LOGFN;
  VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  VkUtils::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                        stagingBufferMemory);

  void* data;
  vkMapMemory(m_Context->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, m_Indices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(m_Context->getDevice(), stagingBufferMemory);

  VkUtils::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer, m_IndexBufferMemory);

  copyBuffer(stagingBuffer, m_IndexBuffer, bufferSize);

  vkDestroyBuffer(m_Context->getDevice(), stagingBuffer, nullptr);
  vkFreeMemory(m_Context->getDevice(), stagingBufferMemory, nullptr);
}

// TODO: Remove this function
void Mesh::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
  LOGFN;
  // auto commandManager = m_Context->getCommandManager();
  // VkCommandBuffer commandBuffer = m_Context->beginSingleTimeCommands();

  // VkBufferCopy copyRegion{};
  // copyRegion.size = size;
  // vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  // m_Context->endSingleTimeCommands(commandBuffer);

  auto commandPool = m_Context->getCommandPool();
  auto device = m_Context->getDevice();

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = 0;  // Optional
  copyRegion.dstOffset = 0;  // Optional
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  auto graphicsQueue = m_Context->getGraphicsQueue();
  vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphicsQueue);

  vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void Mesh::bind(VkCommandBuffer commandBuffer) {
  LOGFN_ONCE;
  VkBuffer buffers[] = {m_VertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

  if (m_HasIndices) {
    vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
  }
}

void Mesh::draw(VkCommandBuffer commandBuffer) {
  LOGFN_ONCE;
  if (m_HasIndices) {
    vkCmdDrawIndexed(commandBuffer, m_IndexCount, 1, 0, 0, 0);
  } else {
    vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0);
  }
}

}  // namespace glint