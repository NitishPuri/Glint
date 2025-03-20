#include "mesh.h"

#include "logger.h"
#include "vulkan_context.h"

namespace glint {

VkVertexInputBindingDescription Vertex::getBindingDescription() {
  LOGFN;
  VkVertexInputBindingDescription bindingDescription{};
  bindingDescription.binding = 0;
  bindingDescription.stride = sizeof(Vertex);
  bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> Vertex::getAttributeDescriptions() {
  LOGFN;
  std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
  attributeDescriptions[0].binding = 0;
  attributeDescriptions[0].location = 0;
  attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  LOGCALL(attributeDescriptions[0].offset = offsetof(Vertex, position));

  attributeDescriptions[1].binding = 0;
  attributeDescriptions[1].location = 1;
  attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  LOGCALL(attributeDescriptions[1].offset = offsetof(Vertex, color));

  return attributeDescriptions;
}

Mesh::Mesh(VulkanContext* context, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    : m_Context(context),
      m_Vertices(vertices),
      m_Indices(indices),
      m_IndexCount(static_cast<uint32_t>(indices.size())),
      m_VertexCount(static_cast<uint32_t>(vertices.size())),
      m_HasIndices(!indices.empty()) {
  LOGFN;
  createVertexBuffer();
  //   if (m_HasIndices) {
  //     createIndexBuffer();
  //   }

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

  //   m_Context->createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
  //                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  //                           m_VertexBuffer, m_VertexBufferMemory);

  //   auto device = m_Context->getDevice();
  //   void* data;
  //   LOGCALL(vkMapMemory(device, m_VertexBufferMemory, 0, bufferSize, 0, &data));
  //   LOGCALL(memcpy(data, m_Vertices.data(), (size_t)bufferSize));
  //   LOGCALL(vkUnmapMemory(device, m_VertexBufferMemory));

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  m_Context->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                          stagingBufferMemory);

  void* data;
  vkMapMemory(m_Context->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, m_Vertices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(m_Context->getDevice(), stagingBufferMemory);

  m_Context->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory);

  copyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

  vkDestroyBuffer(m_Context->getDevice(), stagingBuffer, nullptr);
  vkFreeMemory(m_Context->getDevice(), stagingBufferMemory, nullptr);
}

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

  LOG("Command Buffer for Buffer Copy");
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  LOGCALL(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  LOG("Begin Command Buffer");
  LOGCALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));

  LOG("Copy Buffer");
  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = 0;  // Optional
  copyRegion.dstOffset = 0;  // Optional
  copyRegion.size = size;
  LOGCALL(vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion));

  LOG("End Command Buffer");
  LOGCALL(vkEndCommandBuffer(commandBuffer));

  LOG("Submit Command Buffer");
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  auto graphicsQueue = m_Context->getGraphicsQueue();
  LOGCALL(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
  LOGCALL(vkQueueWaitIdle(graphicsQueue));

  LOG("Free Command Buffer");
  LOGCALL(vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer));
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