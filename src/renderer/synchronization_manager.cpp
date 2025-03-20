#include "synchronization_manager.h"

#include <stdexcept>

#include "../logger.h"
#include "vulkan_context.h"

namespace glint {

SynchronizationManager::SynchronizationManager(VulkanContext* context)
    : m_Context(context),
      m_ImageAvailableSemaphore(VK_NULL_HANDLE),
      m_RenderFinishedSemaphore(VK_NULL_HANDLE),
      m_InFlightFence(VK_NULL_HANDLE) {
  LOGFN;
  createSyncObjects();
}

SynchronizationManager::~SynchronizationManager() {
  LOGFN;
  VkDevice device = m_Context->getDevice();

  if (m_ImageAvailableSemaphore != VK_NULL_HANDLE) {
    vkDestroySemaphore(device, m_ImageAvailableSemaphore, nullptr);
  }

  if (m_RenderFinishedSemaphore != VK_NULL_HANDLE) {
    vkDestroySemaphore(device, m_RenderFinishedSemaphore, nullptr);
  }

  if (m_InFlightFence != VK_NULL_HANDLE) {
    vkDestroyFence(device, m_InFlightFence, nullptr);
  }
}

void SynchronizationManager::createSyncObjects() {
  LOGFN;
  LOG("Synchronization:");
  LOG("Semaphors: signal and wait for the image available and render finished, sync between queues");
  LOG("Fences: wait for the frame to finish before starting the next one, sync between CPU and GPU");

  VkDevice device = m_Context->getDevice();

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  LOG("Create fence in signaled state, so that the first frame can start immediately");
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore) != VK_SUCCESS ||
      vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore) != VK_SUCCESS ||
      vkCreateFence(device, &fenceInfo, nullptr, &m_InFlightFence) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create synchronization objects for a frame!");
  }
}

void SynchronizationManager::waitForFence() {
  LOGFN_ONCE;
  LOG_ONCE("Wait for the previous frame to be finished");
  vkWaitForFences(m_Context->getDevice(), 1, &m_InFlightFence, VK_TRUE, UINT64_MAX);
}

void SynchronizationManager::resetFence() {
  LOGFN_ONCE;
  vkResetFences(m_Context->getDevice(), 1, &m_InFlightFence);
}

}  // namespace glint