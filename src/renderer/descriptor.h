#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace glint {

class VkContext;

class DescriptorSetLayout {
 public:
  // Builder class for descriptor set layout
  class Builder {
   public:
    Builder(VkContext* context) : m_Context(context) {}

    // Add a binding to the descriptor set layout
    Builder& addBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t count = 1);

    // Add uniform buffer binding convenience method
    Builder& addUniformBuffer(uint32_t binding, VkShaderStageFlags stageFlags) {
      return addBinding(binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stageFlags);
    }

    // Add texture sampler binding convenience method
    Builder& addTextureSampler(uint32_t binding, VkShaderStageFlags stageFlags, uint32_t count = 1) {
      return addBinding(binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stageFlags, count);
    }

    // Build and return the descriptor set layout
    std::unique_ptr<DescriptorSetLayout> build() {
      return std::make_unique<DescriptorSetLayout>(m_Context, m_Bindings);
    }

   private:
    VkContext* m_Context;
    std::vector<VkDescriptorSetLayoutBinding> m_Bindings;
  };

  DescriptorSetLayout(VkContext* context, const std::vector<VkDescriptorSetLayoutBinding>& bindings);
  ~DescriptorSetLayout();

  // Prevent copying
  DescriptorSetLayout(const DescriptorSetLayout&) = delete;
  DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

  VkDescriptorSetLayout getLayout() const { return m_Layout; }

 private:
  void createLayout();

  VkContext* m_Context;
  VkDescriptorSetLayout m_Layout = VK_NULL_HANDLE;
  std::vector<VkDescriptorSetLayoutBinding> m_Bindings;
};

class DescriptorPool {
 public:
  DescriptorPool(VkContext* context, uint32_t maxSets);
  ~DescriptorPool();

  // Prevent copying
  DescriptorPool(const DescriptorPool&) = delete;
  DescriptorPool& operator=(const DescriptorPool&) = delete;

  VkDescriptorPool getPool() const { return m_Pool; }

 private:
  VkContext* m_Context;
  VkDescriptorPool m_Pool = VK_NULL_HANDLE;
};

class Descriptor {
 public:
  Descriptor(VkContext* context, DescriptorSetLayout* layout, DescriptorPool* pool, uint32_t count = 1);
  ~Descriptor();

  // Prevent copying
  Descriptor(const Descriptor&) = delete;
  Descriptor& operator=(const Descriptor&) = delete;

  // void updateUniformBuffer(VkBuffer buffer, size_t size);
  // void updateUniformBuffers(std::vector<std::unique_ptr<UniformBuffer>> buffers, size_t size);
  // void updateUniformBuffer(VkBuffer buffer, VkDeviceSize size, VkDeviceSize offset = 0, uint32_t setIndex = 0);
  void updateUniformBuffer(VkBuffer buffer, VkDeviceSize size, VkDeviceSize offset = 0, uint32_t setIndex = 0);

  void bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t setIndex = 0);

  // VkDescriptorSet getDescriptorSet() const { return m_DescriptorSet; }

 private:
  VkContext* m_Context;

  // DescriptorSetLayout* m_Layout;
  // DescriptorPool* m_Pool;
  // VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
  std::vector<VkDescriptorSet> m_DescriptorSets;
};

class UniformBuffer {
 public:
  UniformBuffer(VkContext* context, size_t size);
  ~UniformBuffer();

  // Prevent copying
  UniformBuffer(const UniformBuffer&) = delete;
  UniformBuffer& operator=(const UniformBuffer&) = delete;

  void update(const void* data, size_t size);
  VkBuffer getBuffer() const { return m_Buffer; }

 private:
  VkContext* m_Context;
  VkBuffer m_Buffer = VK_NULL_HANDLE;
  VkDeviceMemory m_Memory = VK_NULL_HANDLE;
  void* m_MappedData = nullptr;
};

}  // namespace glint