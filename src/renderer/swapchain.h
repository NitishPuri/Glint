#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace glint {

class VulkanContext;

class SwapChain {
 public:
  SwapChain(VulkanContext* context);
  ~SwapChain();

  // Prevent copying
  SwapChain(const SwapChain&) = delete;
  SwapChain& operator=(const SwapChain&) = delete;

  // Getters
  VkSwapchainKHR getSwapChain() const { return m_SwapChain; }
  VkFormat getImageFormat() const { return m_ImageFormat; }
  VkExtent2D getExtent() const { return m_Extent; }
  uint32_t getImageCount() const { return static_cast<uint32_t>(m_Images.size()); }
  VkImageView getImageView(size_t index) const { return m_ImageViews[index]; }
  VkFramebuffer getFramebuffer(size_t index) const { return m_Framebuffers[index]; }

  void createFramebuffers(VkRenderPass renderPass);

  void cleanup();
  void recreateSwapchain(VkRenderPass renderPass);

 private:
  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  void createSwapChain();
  void createImageViews();
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

 private:
  VulkanContext* m_Context;

  VkSwapchainKHR m_SwapChain;
  std::vector<VkImage> m_Images;
  VkFormat m_ImageFormat;
  VkExtent2D m_Extent;
  std::vector<VkImageView> m_ImageViews;
  std::vector<VkFramebuffer> m_Framebuffers;
};

}  // namespace glint