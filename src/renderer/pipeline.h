#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace glint {

class VulkanContext;
class SwapChain;
class RenderPass;

class Pipeline {
 public:
  Pipeline(VulkanContext* context, RenderPass* renderPass, const std::string& vertShaderPath,
           const std::string& fragShaderPath);
  ~Pipeline();

  // Prevent copying
  Pipeline(const Pipeline&) = delete;
  Pipeline& operator=(const Pipeline&) = delete;

  // Getters
  VkPipeline getPipeline() const { return m_Pipeline; }
  VkPipelineLayout getPipelineLayout() const { return m_PipelineLayout; }

  void bind(VkCommandBuffer commandBuffer);

 private:
  void createGraphicsPipeline(const std::string& vertShaderPath, const std::string& fragShaderPath);
  VkShaderModule createShaderModule(const std::vector<char>& code);
  std::vector<char> readFile(const std::string& filename);

 private:
  VulkanContext* m_Context;
  RenderPass* m_RenderPass;
  VkPipelineLayout m_PipelineLayout;
  VkPipeline m_Pipeline;
};

}  // namespace glint