#pragma once

#include <vulkan/vulkan.h>

namespace glint {

class VulkanContext;
class SwapChain;

class RenderPass {
 public:
  RenderPass(VulkanContext* context, SwapChain* swapChain);
  ~RenderPass();

  // Prevent copying
  RenderPass(const RenderPass&) = delete;
  RenderPass& operator=(const RenderPass&) = delete;

  // Getters
  VkRenderPass getRenderPass() const { return m_RenderPass; }

  void begin(VkCommandBuffer commandBuffer, uint32_t imageIndex, const VkClearValue& clearValue);
  void end(VkCommandBuffer commandBuffer);

 private:
  void createRenderPass();

 private:
  VulkanContext* m_Context;
  SwapChain* m_SwapChain;
  VkRenderPass m_RenderPass;
};

}  // namespace glint