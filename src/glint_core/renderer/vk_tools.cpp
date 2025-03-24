/*
 * Assorted commonly used Vulkan helper functions
 *
 * Copyright (C) 2016-2024 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#include "vk_tools.h"

namespace glint {
namespace tools {
bool errorModeSilent = false;
std::string errorString(VkResult errorCode) {
  switch (errorCode) {
#define STR(r) \
  case VK_##r: \
    return #r
    STR(NOT_READY);
    STR(TIMEOUT);
    STR(EVENT_SET);
    STR(EVENT_RESET);
    STR(INCOMPLETE);
    STR(ERROR_OUT_OF_HOST_MEMORY);
    STR(ERROR_OUT_OF_DEVICE_MEMORY);
    STR(ERROR_INITIALIZATION_FAILED);
    STR(ERROR_DEVICE_LOST);
    STR(ERROR_MEMORY_MAP_FAILED);
    STR(ERROR_LAYER_NOT_PRESENT);
    STR(ERROR_EXTENSION_NOT_PRESENT);
    STR(ERROR_FEATURE_NOT_PRESENT);
    STR(ERROR_INCOMPATIBLE_DRIVER);
    STR(ERROR_TOO_MANY_OBJECTS);
    STR(ERROR_FORMAT_NOT_SUPPORTED);
    STR(ERROR_SURFACE_LOST_KHR);
    STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
    STR(SUBOPTIMAL_KHR);
    STR(ERROR_OUT_OF_DATE_KHR);
    STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
    STR(ERROR_VALIDATION_FAILED_EXT);
    STR(ERROR_INVALID_SHADER_NV);
    STR(ERROR_INCOMPATIBLE_SHADER_BINARY_EXT);
#undef STR
    default:
      return "UNKNOWN_ERROR";
  }
}

std::string physicalDeviceTypeString(VkPhysicalDeviceType type) {
  switch (type) {
#define STR(r)                      \
  case VK_PHYSICAL_DEVICE_TYPE_##r: \
    return #r
    STR(OTHER);
    STR(INTEGRATED_GPU);
    STR(DISCRETE_GPU);
    STR(VIRTUAL_GPU);
    STR(CPU);
#undef STR
    default:
      return "UNKNOWN_DEVICE_TYPE";
  }
}

}  // namespace tools
}  // namespace glint