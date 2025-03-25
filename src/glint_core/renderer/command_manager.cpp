#include "command_manager.h"

#include <stdexcept>

#include "core/logger.h"
#include "vk_context.h"
#include "vk_utils.h"

namespace glint {

CommandManager::CommandManager(VkContext* context)
    : m_Context(context), m_CmdBufferCount(0), m_CommandPool(VK_NULL_HANDLE) {
  createCommandPool();
  m_Context->setCommandPool(m_CommandPool);
}

CommandManager::CommandManager(VkContext* context, uint32_t cmdBufferCount)
    : m_Context(context), m_CmdBufferCount(cmdBufferCount), m_CommandPool(VK_NULL_HANDLE) {
  createCommandPool();
  createCommandBuffers();
  m_Context->setCommandPool(m_CommandPool);
}

CommandManager::~CommandManager() {
  if (m_CommandPool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(m_Context->getDevice(), m_CommandPool, nullptr);
  }
}

void CommandManager::createCommandPool() {
  auto queueFamilyIndices = m_Context->getQueueFamilyIndices();

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

  VK_CHECK_RESULT(vkCreateCommandPool(m_Context->getDevice(), &poolInfo, nullptr, &m_CommandPool));
}

void CommandManager::createCommandBuffers() {
  m_CommandBuffers.resize(m_CmdBufferCount);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = m_CommandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = m_CmdBufferCount;

  VK_CHECK_RESULT(vkAllocateCommandBuffers(m_Context->getDevice(), &allocInfo, m_CommandBuffers.data()));
}

void CommandManager::beginSingleTimeCommands(uint32_t frameIndex) {
  assert(frameIndex < m_CommandBuffers.size());

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0;                   // Optional
  beginInfo.pInheritanceInfo = nullptr;  // Optional

  VK_CHECK_RESULT(vkBeginCommandBuffer(m_CommandBuffers[frameIndex], &beginInfo));
}

void CommandManager::endSingleTimeCommands(uint32_t frameIndex) {
  assert(frameIndex < m_CommandBuffers.size());
  VK_CHECK_RESULT(vkEndCommandBuffer(m_CommandBuffers[frameIndex]));
}

void CommandManager::resetCommandBuffer(uint32_t frameIndex) {
  assert(frameIndex < m_CommandBuffers.size());
  VK_CHECK_RESULT(vkResetCommandBuffer(m_CommandBuffers[frameIndex], 0));
}

}  // namespace glint