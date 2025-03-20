#pragma once

#include <vulkan/vulkan.h>

namespace glint {

class VulkanContext;

class SynchronizationManager {
 public:
  SynchronizationManager(VulkanContext* context);
  ~SynchronizationManager();

  // Prevent copying
  SynchronizationManager(const SynchronizationManager&) = delete;
  SynchronizationManager& operator=(const SynchronizationManager&) = delete;

  // Getters
  VkSemaphore getImageAvailableSemaphore() const { return m_ImageAvailableSemaphore; }
  VkSemaphore getRenderFinishedSemaphore() const { return m_RenderFinishedSemaphore; }
  VkFence getInFlightFence() const { return m_InFlightFence; }

  // Synchronization operations
  void waitForFence();
  void resetFence();

 private:
  void createSyncObjects();

 private:
  VulkanContext* m_Context;
  VkSemaphore m_ImageAvailableSemaphore;
  VkSemaphore m_RenderFinishedSemaphore;
  VkFence m_InFlightFence;
};

}  // namespace glint