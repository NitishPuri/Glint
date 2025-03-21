#pragma once

#include <vulkan/vulkan.h>

#include <string>

namespace glint {

class VkContext;

class VkUtils {
 public:
  // Initialize with context
  static void init(VkContext* context);
  static void cleanup();

  static void setObjectName(uint64_t object, VkObjectType objectType, const char* name);

  template <typename T>
  static void setObjectName(T handle, VkObjectType objectType, const char* name) {
    setObjectName((uint64_t)(uint64_t)handle, objectType, name);
  }

  // Buffer operations
  static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                           VkBuffer& buffer, VkDeviceMemory& bufferMemory);

  static void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

  // Image operations
  static void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                          VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image,
                          VkDeviceMemory& imageMemory);

  static void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

  static void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

  // Image view creation
  static VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

 private:
  static VkContext* s_Context;

  // Helper for command buffer management
  static VkCommandBuffer beginSingleTimeCommands();
  static void endSingleTimeCommands(VkCommandBuffer commandBuffer);
};

}  // namespace glint