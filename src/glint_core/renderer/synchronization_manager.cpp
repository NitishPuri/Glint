#include "synchronization_manager.h"

#include <stdexcept>

#include "core/logger.h"
#include "vk_context.h"
#include "vk_tools.h"

namespace glint {

SynchronizationManager::SynchronizationManager(VkContext* context, uint32_t maxFramesInFlight)
    : m_Context(context), m_MaxFramesInFlight(maxFramesInFlight) {
  LOGFN;
  createSyncObjects();
}

SynchronizationManager::~SynchronizationManager() {
  LOGFN;
  VkDevice device = m_Context->getDevice();

  for (uint32_t i = 0; i < m_MaxFramesInFlight; i++) {
    if (m_ImageAvailableSemaphore[i] != VK_NULL_HANDLE) {
      vkDestroySemaphore(device, m_ImageAvailableSemaphore[i], nullptr);
    }

    if (m_RenderFinishedSemaphore[i] != VK_NULL_HANDLE) {
      vkDestroySemaphore(device, m_RenderFinishedSemaphore[i], nullptr);
    }

    if (m_InFlightFence[i] != VK_NULL_HANDLE) {
      vkDestroyFence(device, m_InFlightFence[i], nullptr);
    }
  }
}

void SynchronizationManager::createSyncObjects() {
  LOGFN;
  LOG("Synchronization:");
  LOG("Semaphors: signal and wait for the image available and render finished, sync between queues");
  LOG("Fences: wait for the frame to finish before starting the next one, sync between CPU and GPU");
  LOG("Creating", m_MaxFramesInFlight, "sets of synchronization objects");

  m_ImageAvailableSemaphore.resize(m_MaxFramesInFlight);
  m_RenderFinishedSemaphore.resize(m_MaxFramesInFlight);
  m_InFlightFence.resize(m_MaxFramesInFlight);

  VkDevice device = m_Context->getDevice();

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  LOG("Create fence in signaled state, so that the first frame can start immediately");
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < m_MaxFramesInFlight; i++) {
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore[i]) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore[i]) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, nullptr, &m_InFlightFence[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create synchronization objects for a frame!");
    }
  }
}

void SynchronizationManager::waitForFence(uint32_t frameIndex) {
  LOGFN_ONCE;
  // TODO: Choose between assrts, exceptions, or return codes
  if (frameIndex >= m_MaxFramesInFlight) {
    throw std::runtime_error("Frame index out of range");
  }

  LOG_ONCE("Wait for the previous frame to be finished");
  vkWaitForFences(m_Context->getDevice(), 1, &m_InFlightFence[frameIndex], VK_TRUE, DEFAULT_FENCE_TIMEOUT);
}

void SynchronizationManager::resetFence(uint32_t frameIndex) {
  LOGFN_ONCE;
  if (frameIndex >= m_MaxFramesInFlight) {
    throw std::runtime_error("Frame index out of range");
  }
  vkResetFences(m_Context->getDevice(), 1, &m_InFlightFence[frameIndex]);
}

}  // namespace glint