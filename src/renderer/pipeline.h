#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace glint {

class VkContext;
class SwapChain;
class RenderPass;

class DescriptorSetLayout;
// class DescriptorPool;

class Pipeline {
 public:
  Pipeline(VkContext* context, RenderPass* renderPass, const std::string& vertShaderPath,
           const std::string& fragShaderPath, DescriptorSetLayout* descriptorSetLayout = nullptr);
  ~Pipeline();

  // Prevent copying
  Pipeline(const Pipeline&) = delete;
  Pipeline& operator=(const Pipeline&) = delete;

  // Getters
  VkPipeline getPipeline() const { return m_Pipeline; }
  VkPipelineLayout getPipelineLayout() const { return m_PipelineLayout; }

  void bind(VkCommandBuffer commandBuffer);

  // void setDescriptorSetLayout(DescriptorSetLayout* layout) { m_DescriptorSetLayout = layout; }

 private:
  void createGraphicsPipeline(const std::string& vertShaderPath, const std::string& fragShaderPath);
  VkShaderModule createShaderModule(const std::vector<char>& code);
  std::vector<char> readFile(const std::string& filename);

 private:
  VkContext* m_Context;
  RenderPass* m_RenderPass;
  VkPipelineLayout m_PipelineLayout;
  VkPipeline m_Pipeline;

  DescriptorSetLayout* m_DescriptorSetLayout = nullptr;
  // VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
  // DescriptorPool* m_DescriptorPool;
};

}  // namespace glint