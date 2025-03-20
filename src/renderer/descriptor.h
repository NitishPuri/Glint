#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace glint {

class VulkanContext;

class DescriptorSetLayout {
 public:
  DescriptorSetLayout(VulkanContext* context);
  ~DescriptorSetLayout();

  // Prevent copying
  DescriptorSetLayout(const DescriptorSetLayout&) = delete;
  DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

  VkDescriptorSetLayout getLayout() const { return m_Layout; }

 private:
  VulkanContext* m_Context;
  VkDescriptorSetLayout m_Layout = VK_NULL_HANDLE;
};

class DescriptorPool {
 public:
  DescriptorPool(VulkanContext* context, uint32_t maxSets = 1000);
  ~DescriptorPool();

  // Prevent copying
  DescriptorPool(const DescriptorPool&) = delete;
  DescriptorPool& operator=(const DescriptorPool&) = delete;

  VkDescriptorPool getPool() const { return m_Pool; }

 private:
  VulkanContext* m_Context;
  VkDescriptorPool m_Pool = VK_NULL_HANDLE;
};

class UniformBuffer {
 public:
  UniformBuffer(VulkanContext* context, size_t size);
  ~UniformBuffer();

  // Prevent copying
  UniformBuffer(const UniformBuffer&) = delete;
  UniformBuffer& operator=(const UniformBuffer&) = delete;

  void update(const void* data, size_t size);
  VkBuffer getBuffer() const { return m_Buffer; }

 private:
  VulkanContext* m_Context;
  VkBuffer m_Buffer = VK_NULL_HANDLE;
  VkDeviceMemory m_Memory = VK_NULL_HANDLE;
  void* m_MappedData = nullptr;
};

}  // namespace glint