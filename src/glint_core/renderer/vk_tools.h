/*
 * Assorted Vulkan helper functions
 *
 * Copyright (C) 2016-2023 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#pragma once

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "initializers.h"
#include "vulkan/vulkan.h"

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#endif

// Custom define for better code readability
#define VK_FLAGS_NONE 0
// Default fence timeout in nanoseconds
#define DEFAULT_FENCE_TIMEOUT 100000000000

// Macro to check and display Vulkan return results
#define VK_CHECK_RESULT(f)                                                                                           \
  {                                                                                                                  \
    VkResult res = (f);                                                                                              \
    if (res != VK_SUCCESS) {                                                                                         \
      std::cout << "Fatal : VkResult is \"" << glint::tools::errorString(res) << "\" in " << __FILE__ << " at line " \
                << __LINE__ << "\n";                                                                                 \
      assert(res == VK_SUCCESS);                                                                                     \
    }                                                                                                                \
  }

// const std::string getAssetPath();
// const std::string getShaderBasePath();

namespace glint {
namespace tools {
/** @brief Setting this path chnanges the place where the samples looks for assets and shaders */
// extern std::string resourcePath;

/** @brief Disable message boxes on fatal errors */
extern bool errorModeSilent;

/** @brief Returns an error code as a string */
std::string errorString(VkResult errorCode);

/** @brief Returns the device type as a string */
std::string physicalDeviceTypeString(VkPhysicalDeviceType type);

// Selected a suitable supported depth format starting with 32 bit down to 16 bit
// Returns false if none of the depth formats in the list is supported by the device
// VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat* depthFormat);
// Same as getSupportedDepthFormat but will only select formats that also have stencil
// VkBool32 getSupportedDepthStencilFormat(VkPhysicalDevice physicalDevice, VkFormat* depthStencilFormat);

// // Returns true a given format support LINEAR filtering
// VkBool32 formatIsFilterable(VkPhysicalDevice physicalDevice, VkFormat format, VkImageTiling tiling);
// // Returns true if a given format has a stencil part
// VkBool32 formatHasStencil(VkFormat format);

// // Put an image memory barrier for setting an image layout on the sub resource into the given command buffer
// void setImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkImageLayout oldImageLayout,
//                     VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange,
//                     VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
//                     VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
// // Uses a fixed sub resource layout with first mip level and layer
// void setImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkImageAspectFlags aspectMask,
//                     VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
//                     VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
//                     VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

// /** @brief Insert an image memory barrier into the command buffer */
// void insertImageMemoryBarrier(VkCommandBuffer cmdbuffer, VkImage image, VkAccessFlags srcAccessMask,
//                               VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout
//                               newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
//                               VkImageSubresourceRange subresourceRange);

// // Display error message and exit on fatal error
// void exitFatal(const std::string& message, int32_t exitCode);
// void exitFatal(const std::string& message, VkResult resultCode);

// // Load a SPIR-V shader (binary)
// #if defined(__ANDROID__)
// VkShaderModule loadShader(AAssetManager* assetManager, const char* fileName, VkDevice device);
// #else
// VkShaderModule loadShader(const char* fileName, VkDevice device);
// #endif

// /** @brief Checks if a file exists */
// bool fileExists(const std::string& filename);

// uint32_t alignedSize(uint32_t value, uint32_t alignment);
// VkDeviceSize alignedVkSize(VkDeviceSize value, VkDeviceSize alignment);
}  // namespace tools
}  // namespace glint
