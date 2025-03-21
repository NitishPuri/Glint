#include "mesh.h"

#include "logger.h"
#include "vk_context.h"
#include "vk_utils.h"

namespace glint {

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