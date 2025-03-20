#include "swapchain.h"

#include <algorithm>
#include <stdexcept>

#include "core/window.h"
#include "logger.h"
#include "vulkan_context.h"

namespace glint {

SwapChain::SwapChain(VulkanContext* context) : m_Context(context), m_SwapChain(VK_NULL_HANDLE) {
  LOGFN;
  createSwapChain();
  createImageViews();
}

SwapChain::~SwapChain() {
  LOGFN;
  cleanup();
}

void SwapChain::cleanup() {
  LOGFN;
  VkDevice device = m_Context->getDevice();

  LOG("Destroying ", m_Framebuffers.size(), " framebuffers");
  for (auto framebuffer : m_Framebuffers) {
    vkDestroyFramebuffer(device, framebuffer, nullptr);
  }

  LOG("Destroying ", m_ImageViews.size(), " image views");
  for (auto imageView : m_ImageViews) {
    vkDestroyImageView(device, imageView, nullptr);
  }

  LOGCALL(vkDestroySwapchainKHR(device, m_SwapChain, nullptr));
}

void SwapChain::recreateSwapchain(VkRenderPass renderPass) {
  LOGFN;

  VkDevice device = m_Context->getDevice();
  LOGCALL(vkDeviceWaitIdle(device));

  cleanup();

  createSwapChain();
  createImageViews();
  createFramebuffers(renderPass);
}

void SwapChain::createSwapChain() {
  LOGFN;
  SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_Context->getPhysicalDevice());

  VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

  // Request at least one more image than the minimum
  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = m_Context->getSurface();
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;  // Always 1 unless stereoscopic 3D
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  // Handle queue family sharing
  VulkanContext::QueueFamilyIndices indices = m_Context->getQueueFamilyIndices();
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily) {
    LOG("graphics and present queues are different");
    LOG("Using concurrent sharing mode");
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    LOG("graphics and present queues are the same");
    LOG("Using exclusive sharing mode");
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;
  // QUESTION: What is the purpose of oldSwapChain?
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(m_Context->getDevice(), &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create swap chain!");
  }

  // Get swap chain images
  vkGetSwapchainImagesKHR(m_Context->getDevice(), m_SwapChain, &imageCount, nullptr);
  m_Images.resize(imageCount);
  vkGetSwapchainImagesKHR(m_Context->getDevice(), m_SwapChain, &imageCount, m_Images.data());

  // Store format and extent
  m_ImageFormat = surfaceFormat.format;
  m_Extent = extent;
}

void SwapChain::createImageViews() {
  LOGFN;
  m_ImageViews.resize(m_Images.size());

  for (size_t i = 0; i < m_Images.size(); i++) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = m_Images[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = m_ImageFormat;

    // Default color mapping
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    // The image will be used as a color attachment
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_Context->getDevice(), &createInfo, nullptr, &m_ImageViews[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create image views!");
    }
  }
}

void SwapChain::createFramebuffers(VkRenderPass renderPass) {
  LOGFN;
  m_Framebuffers.resize(m_ImageViews.size());

  for (size_t i = 0; i < m_ImageViews.size(); i++) {
    VkImageView attachments[] = {m_ImageViews[i]};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = m_Extent.width;
    framebufferInfo.height = m_Extent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(m_Context->getDevice(), &framebufferInfo, nullptr, &m_Framebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create framebuffer!");
    }
  }
}

SwapChain::SwapChainSupportDetails SwapChain::querySwapChainSupport(VkPhysicalDevice device) {
  LOGFN;
  SwapChainSupportDetails details;
  VkSurfaceKHR surface = m_Context->getSurface();

  // Get surface capabilities
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

  // Get surface formats
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
  }

  // Get presentation modes
  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
  }

  return details;
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
  LOGFN;
  // Prefer SRGB color space with B8G8R8A8 format
  for (const auto& availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  // If our preferred format isn't available, just use the first one
  return availableFormats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
  LOGFN;
  // Prefer mailbox (triple buffering) if available
  for (const auto& availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      LOG("Using VK_PRESENT_MODE_MAILBOX_KHR (triple buffering)");
      return availablePresentMode;
    }
  }

  // Fall back to FIFO (guaranteed to be available, similar to vsync)
  LOG("Using VK_PRESENT_MODE_FIFO_KHR (vsync)");
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
  LOGFN;
  if (capabilities.currentExtent.width != UINT32_MAX) {
    // If the surface size is defined, we must match it
    return capabilities.currentExtent;
  } else {
    // Otherwise, we need to pick a size within the allowed bounds
    // First, get the actual window size from the Window class through VulkanContext
    Window* window = dynamic_cast<Window*>(m_Context->getWindow());
    uint32_t width = window->getWidth();
    uint32_t height = window->getHeight();

    LOG("Window size: ", width, "x", height);

    VkExtent2D actualExtent = {width, height};

    // Clamp to the allowed minimum and maximum extents
    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
  }
}

}  // namespace glint