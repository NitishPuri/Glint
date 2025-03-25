#include "synchronization_manager.h"

#include <stdexcept>

#include "core/logger.h"
#include "vk_context.h"
#include "vk_tools.h"

namespace glint {

SynchronizationManager::SynchronizationManager(VkContext* context, uint32_t maxFramesInFlight, uint32_t swapImageCount)
    : m_Context(context), m_MaxFramesInFlight(maxFramesInFlight), m_SwapImageCount(swapImageCount) {
  LOGFN;
  createSyncObjects();
}

SynchronizationManager::~SynchronizationManager() {
  LOGFN;
  VkDevice device = m_Context->getDevice();

  for (auto semaphore : m_ImageAvailableSemaphore) {
    if (semaphore != VK_NULL_HANDLE) {
      vkDestroySemaphore(device, semaphore, nullptr);
    }
  }

  for (auto semaphore : m_RenderFinishedSemaphore) {
    if (semaphore != VK_NULL_HANDLE) {
      vkDestroySemaphore(device, semaphore, nullptr);
    }
  }

  for (auto fence : m_InFlightFence) {
    if (fence != VK_NULL_HANDLE) {
      vkDestroyFence(device, fence, nullptr);
    }
  }
}

void SynchronizationManager::createSyncObjects() {
  m_ImageAvailableSemaphore.resize(m_MaxFramesInFlight);
  m_RenderFinishedSemaphore.resize(m_MaxFramesInFlight);

  VkDevice device = m_Context->getDevice();

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  for (size_t i = 0; i < m_MaxFramesInFlight; i++) {
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore[i]));
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore[i]));
  }

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  m_InFlightFence.resize(m_SwapImageCount);
  for (size_t i = 0; i < m_SwapImageCount; i++) {
    VK_CHECK_RESULT(vkCreateFence(device, &fenceInfo, nullptr, &m_InFlightFence[i]));
  }
}

void SynchronizationManager::waitForFence(uint32_t frameIndex) {
  LOGFN_ONCE;
  // TODO: Choose between assrts, exceptions, or return codes
  assert(frameIndex < m_SwapImageCount);

  VK_CHECK_RESULT(
      vkWaitForFences(m_Context->getDevice(), 1, &m_InFlightFence[frameIndex], VK_TRUE, DEFAULT_FENCE_TIMEOUT));
}

void SynchronizationManager::resetFence(uint32_t frameIndex) {
  LOGFN_ONCE;
  assert(frameIndex < m_SwapImageCount);
  VK_CHECK_RESULT(vkResetFences(m_Context->getDevice(), 1, &m_InFlightFence[frameIndex]));
}

}  // namespace glint