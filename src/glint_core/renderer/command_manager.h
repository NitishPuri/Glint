#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace glint {

class VkContext;

class CommandManager {
 public:
  CommandManager(VkContext* context);
  CommandManager(VkContext* context, uint32_t maxFramesInFlight);
  ~CommandManager();

  // Prevent copying
  CommandManager(const CommandManager&) = delete;
  CommandManager& operator=(const CommandManager&) = delete;

  // Getters
  VkCommandPool getCommandPool() const { return m_CommandPool; }
  VkCommandBuffer getCommandBuffer(uint32_t frameIndex) const {
    return frameIndex < m_CommandBuffers.size() ? m_CommandBuffers[frameIndex] : VK_NULL_HANDLE;
  }

  void setupCommandBuffers(uint32_t maxFramesInFlight) {
    m_CmdBufferCount = maxFramesInFlight;
    createCommandBuffers();
  }

  // Command buffer operations
  void beginSingleTimeCommands(uint32_t frameIndex);
  void endSingleTimeCommands(uint32_t frameIndex);
  void resetCommandBuffer(uint32_t frameIndex);

 private:
  void createCommandPool();
  void createCommandBuffers();

 private:
  VkContext* m_Context;
  VkCommandPool m_CommandPool;

  std::vector<VkCommandBuffer> m_CommandBuffers;
  uint32_t m_CmdBufferCount;
};

}  // namespace glint