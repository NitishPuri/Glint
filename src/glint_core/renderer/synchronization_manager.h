#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace glint {

class VkContext;

class SynchronizationManager {
 public:
  SynchronizationManager(VkContext* context, uint32_t maxFramesInFlight);
  ~SynchronizationManager();

  // Prevent copying
  SynchronizationManager(const SynchronizationManager&) = delete;
  SynchronizationManager& operator=(const SynchronizationManager&) = delete;

  // Getters
  VkSemaphore getImageAvailableSemaphore(uint32_t frameIndex) const { return m_ImageAvailableSemaphore[frameIndex]; }
  VkSemaphore getRenderFinishedSemaphore(uint32_t frameIndex) const { return m_RenderFinishedSemaphore[frameIndex]; }
  VkFence getInFlightFence(uint32_t frameIndex) const { return m_InFlightFence[frameIndex]; }

  // Synchronization operations
  void waitForFence(uint32_t frameIndex);
  void resetFence(uint32_t frameIndex);

  // Get maximum frames in flight
  //   uint32_t getMaxFramesInFlight() const { return m_MaxFramesInFlight; }

 private:
  void createSyncObjects();

 private:
  VkContext* m_Context;

  uint32_t m_MaxFramesInFlight;

  std::vector<VkSemaphore> m_ImageAvailableSemaphore;
  std::vector<VkSemaphore> m_RenderFinishedSemaphore;
  std::vector<VkFence> m_InFlightFence;
};

}  // namespace glint