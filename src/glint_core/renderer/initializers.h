#include <vulkan/vulkan.h>

#include <vector>

#include "core/common.h"

namespace glint {

namespace initializers {

inline VkDescriptorPoolSize descriptorPoolSize(VkDescriptorType type, uint32_t descriptorCount) {
  VkDescriptorPoolSize descriptorPoolSize{};
  descriptorPoolSize.type = type;
  descriptorPoolSize.descriptorCount = descriptorCount;
  return descriptorPoolSize;
}

inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(uint32_t poolSizeCount, VkDescriptorPoolSize* pPoolSizes,
                                                           uint32_t maxSets) {
  VkDescriptorPoolCreateInfo descriptorPoolInfo{};
  descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolInfo.poolSizeCount = poolSizeCount;
  descriptorPoolInfo.pPoolSizes = pPoolSizes;
  descriptorPoolInfo.maxSets = maxSets;
  return descriptorPoolInfo;
}

inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(const std::vector<VkDescriptorPoolSize>& poolSizes,
                                                           uint32_t maxSets) {
  VkDescriptorPoolCreateInfo descriptorPoolInfo{};
  descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  descriptorPoolInfo.pPoolSizes = poolSizes.data();
  descriptorPoolInfo.maxSets = maxSets;
  return descriptorPoolInfo;
}

}  // namespace initializers
}  // namespace glint