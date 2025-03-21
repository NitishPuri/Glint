#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace glint {

class VkContext;

class SwapChain {
 public:
  SwapChain(VkContext* context);
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

  VkFormat getDepthFormat() const { return m_DepthFormat; }

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

  void createDepthResources();
  void cleanupDepthResources();
  VkFormat findDepthFormat();
  bool hasStencilComponent(VkFormat format);

  VkFormat findSupportedFormats(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                                VkFormatFeatureFlags features);

 private:
  VkContext* m_Context;

  VkSwapchainKHR m_SwapChain;
  std::vector<VkImage> m_Images;
  VkFormat m_ImageFormat;
  VkExtent2D m_Extent;
  std::vector<VkImageView> m_ImageViews;

  std::vector<VkFramebuffer> m_Framebuffers;

  // Depth resources
  VkImage m_DepthImage = VK_NULL_HANDLE;
  VkDeviceMemory m_DepthImageMemory = VK_NULL_HANDLE;
  VkImageView m_DepthImageView = VK_NULL_HANDLE;
  VkFormat m_DepthFormat = VK_FORMAT_UNDEFINED;
};

}  // namespace glint