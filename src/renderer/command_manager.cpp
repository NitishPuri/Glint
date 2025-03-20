#include "command_manager.h"

#include <stdexcept>

#include "logger.h"
#include "vk_context.h"

namespace glint {

CommandManager::CommandManager(VkContext* context, uint32_t maxFramesInFlight)
    : m_Context(context), m_maxFramesInFlight(maxFramesInFlight), m_CommandPool(VK_NULL_HANDLE) {
  LOGFN;
  createCommandPool();
  createCommandBuffers();
}

CommandManager::~CommandManager() {
  LOGFN;
  if (m_CommandPool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(m_Context->getDevice(), m_CommandPool, nullptr);
  }
}

void CommandManager::createCommandPool() {
  LOGFN;
  auto queueFamilyIndices = m_Context->getQueueFamilyIndices();

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  LOG("Choose graphics family as we are using the command buffer for rendering");
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

  if (vkCreateCommandPool(m_Context->getDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create command pool!");
  }
}

void CommandManager::createCommandBuffers() {
  LOGFN;
  LOG("Creating", m_maxFramesInFlight, "command buffers");

  m_CommandBuffers.resize(m_maxFramesInFlight);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = m_CommandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = m_maxFramesInFlight;

  if (vkAllocateCommandBuffers(m_Context->getDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate command buffers!");
  }
}

void CommandManager::beginSingleTimeCommands(uint32_t frameIndex) {
  LOGFN_ONCE;
  if (frameIndex >= m_CommandBuffers.size()) {
    throw std::runtime_error("Frame index out of bounds: " + std::to_string(frameIndex));
  }

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0;                   // Optional
  beginInfo.pInheritanceInfo = nullptr;  // Optional

  if (vkBeginCommandBuffer(m_CommandBuffers[frameIndex], &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("Failed to begin recording command buffer!");
  }
}

void CommandManager::endSingleTimeCommands(uint32_t frameIndex) {
  LOGFN_ONCE;
  if (frameIndex >= m_CommandBuffers.size()) {
    throw std::runtime_error("Frame index out of bounds: " + std::to_string(frameIndex));
  }

  if (vkEndCommandBuffer(m_CommandBuffers[frameIndex]) != VK_SUCCESS) {
    throw std::runtime_error("Failed to record command buffer!");
  }
}

void CommandManager::resetCommandBuffer(uint32_t frameIndex) {
  LOGFN_ONCE;
  if (frameIndex >= m_CommandBuffers.size()) {
    throw std::runtime_error("Frame index out of bounds: " + std::to_string(frameIndex));
  }
  vkResetCommandBuffer(m_CommandBuffers[frameIndex], 0);
}

}  // namespace glint