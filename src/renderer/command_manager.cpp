#include "command_manager.h"

#include <stdexcept>

#include "logger.h"
#include "vulkan_context.h"

namespace glint {

CommandManager::CommandManager(VulkanContext* context)
    : m_Context(context), m_CommandPool(VK_NULL_HANDLE), m_CommandBuffer(VK_NULL_HANDLE) {
  LOGFN;
  createCommandPool();
  createCommandBuffer();
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

void CommandManager::createCommandBuffer() {
  LOGFN;

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = m_CommandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = 1;

  if (vkAllocateCommandBuffers(m_Context->getDevice(), &allocInfo, &m_CommandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate command buffers!");
  }
}

void CommandManager::beginSingleTimeCommands() {
  LOGFN_ONCE;

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0;                   // Optional
  beginInfo.pInheritanceInfo = nullptr;  // Optional

  if (vkBeginCommandBuffer(m_CommandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("Failed to begin recording command buffer!");
  }
}

void CommandManager::endSingleTimeCommands() {
  LOGFN_ONCE;

  if (vkEndCommandBuffer(m_CommandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("Failed to record command buffer!");
  }
}

void CommandManager::reset() {
  LOGFN_ONCE;
  vkResetCommandBuffer(m_CommandBuffer, 0);
}

}  // namespace glint