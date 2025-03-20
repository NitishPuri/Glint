#include "descriptor.h"

#include <stdexcept>

#include "../logger.h"
#include "vk_context.h"

namespace glint {

DescriptorSetLayout::DescriptorSetLayout(VkContext* context) : m_Context(context) {
  LOGFN;

  // Create a descriptor set layout binding for a uniform buffer
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = 0;  // Binding point in the shader
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.descriptorCount = 1;                      // Number of descriptors
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;  // Which shader stages will use this
  uboLayoutBinding.pImmutableSamplers = nullptr;             // Optional for image sampling

  // Create the descriptor set layout
  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = 1;
  layoutInfo.pBindings = &uboLayoutBinding;

  if (vkCreateDescriptorSetLayout(m_Context->getDevice(), &layoutInfo, nullptr, &m_Layout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor set layout!");
  }

  LOG("Created descriptor set layout with UBO binding");
}

DescriptorSetLayout::~DescriptorSetLayout() {
  LOGFN;
  if (m_Layout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(m_Context->getDevice(), m_Layout, nullptr);
  }
}

// DescriptorPool implementation
DescriptorPool::DescriptorPool(VkContext* context, uint32_t maxSets) : m_Context(context) {
  LOGFN;

  // Define which descriptor types the descriptor pool can allocate
  VkDescriptorPoolSize poolSize{};
  poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSize.descriptorCount = maxSets;  // Maximum number of descriptor sets

  // Create the descriptor pool
  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = 1;
  poolInfo.pPoolSizes = &poolSize;
  poolInfo.maxSets = maxSets;  // Maximum number of descriptor sets that can be allocated
  poolInfo.flags = 0;  // Optional: VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT allows individual sets to be freed

  if (vkCreateDescriptorPool(m_Context->getDevice(), &poolInfo, nullptr, &m_Pool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor pool!");
  }

  LOG("Created descriptor pool with capacity for", maxSets, "uniform buffer descriptors");
}

DescriptorPool::~DescriptorPool() {
  LOGFN;
  if (m_Pool != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(m_Context->getDevice(), m_Pool, nullptr);
  }
}

// UniformBuffer implementation
UniformBuffer::UniformBuffer(VkContext* context, size_t size) : m_Context(context), m_MappedData(nullptr) {
  LOGFN;

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(m_Context->getDevice(), &bufferInfo, nullptr, &m_Buffer) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create uniform buffer!");
  }

  // Allocate memory for the buffer
  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(m_Context->getDevice(), m_Buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  // Host visible so we can map it, and host coherent so we don't need to flush explicitly
  allocInfo.memoryTypeIndex = m_Context->findMemoryType(
      memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  if (vkAllocateMemory(m_Context->getDevice(), &allocInfo, nullptr, &m_Memory) != VK_SUCCESS) {
    vkDestroyBuffer(m_Context->getDevice(), m_Buffer, nullptr);
    throw std::runtime_error("Failed to allocate uniform buffer memory!");
  }

  vkBindBufferMemory(m_Context->getDevice(), m_Buffer, m_Memory, 0);

  // Map memory persistently for the lifetime of the buffer
  if (vkMapMemory(m_Context->getDevice(), m_Memory, 0, size, 0, &m_MappedData) != VK_SUCCESS) {
    throw std::runtime_error("Failed to map uniform buffer memory!");
  }

  LOG("Created uniform buffer of size", size, "bytes");
}

UniformBuffer::~UniformBuffer() {
  LOGFN;
  if (m_MappedData) {
    vkUnmapMemory(m_Context->getDevice(), m_Memory);
  }

  if (m_Buffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(m_Context->getDevice(), m_Buffer, nullptr);
  }

  if (m_Memory != VK_NULL_HANDLE) {
    vkFreeMemory(m_Context->getDevice(), m_Memory, nullptr);
  }
}

void UniformBuffer::update(const void* data, size_t size) {
  LOGFN_ONCE;
  if (m_MappedData) {
    memcpy(m_MappedData, data, size);
  }
}

}  // namespace glint