#include "descriptor.h"

#include <stdexcept>

#include "../logger.h"
#include "vk_context.h"
#include "vk_utils.h"

namespace glint {

///////////////////////////////////////////////////////////////////////////
// DescriptorSetLayout

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

///////////////////////////////////////////////////////////////////////////
// DescriptorPool

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

///////////////////////////////////////////////////////////////////////////
// Descriptor

// Add to descriptor.cpp file
Descriptor::Descriptor(VkContext* context, DescriptorSetLayout* layout, DescriptorPool* pool, uint32_t count)
    : m_Context(context) {
  LOGFN;

  // Resize the descriptor sets vector
  m_DescriptorSets.resize(count, VK_NULL_HANDLE);

  // Create descriptor sets
  std::vector<VkDescriptorSetLayout> layouts(count, layout->getLayout());

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = pool->getPool();
  allocInfo.descriptorSetCount = count;
  allocInfo.pSetLayouts = layouts.data();

  if (vkAllocateDescriptorSets(m_Context->getDevice(), &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate descriptor sets!");
  }

  LOG("Allocated", count, "descriptor sets");
}

Descriptor::~Descriptor() {
  LOGFN;
  // No need to explicitly free descriptor sets - they'll be freed when the pool is destroyed
}

void Descriptor::updateUniformBuffer(VkBuffer buffer, VkDeviceSize size, VkDeviceSize offset, uint32_t setIndex) {
  LOGFN_ONCE;
  if (setIndex >= m_DescriptorSets.size()) {
    throw std::runtime_error("Descriptor set index out of bounds!");
  }

  // Define the descriptor buffer info
  VkDescriptorBufferInfo bufferInfo{};
  bufferInfo.buffer = buffer;
  bufferInfo.offset = offset;
  bufferInfo.range = size;

  // Define how to update the descriptor set
  VkWriteDescriptorSet descriptorWrite{};
  descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet = m_DescriptorSets[setIndex];
  descriptorWrite.dstBinding = 0;  // Binding point in the shader
  descriptorWrite.dstArrayElement = 0;
  descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.pBufferInfo = &bufferInfo;

  // Update the descriptor set
  vkUpdateDescriptorSets(m_Context->getDevice(), 1, &descriptorWrite, 0, nullptr);
}

// void Descriptor::bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t setIndex) {
//   LOGFN_ONCE;
//   vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, setIndex, 1,
//                           &m_DescriptorSets[setIndex], 0, nullptr);
// }

void Descriptor::bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t setIndex) {
  LOGFN_ONCE;
  if (setIndex >= m_DescriptorSets.size()) {
    throw std::runtime_error("Descriptor set index out of bounds!");
  }

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                          0,  // First set
                          1,  // Set count
                          &m_DescriptorSets[setIndex], 0, nullptr);
}

///////////////////////////////////////////////////////////////////////////
// UniformBuffer

UniformBuffer::UniformBuffer(VkContext* context, size_t size) : m_Context(context), m_MappedData(nullptr) {
  LOGFN;

  VkUtils::createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_Buffer, m_Memory);

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