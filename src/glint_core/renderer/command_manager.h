#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace glint {

class VkContext;

class CommandManager {
 public:
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

  // Command buffer operations
  void beginSingleTimeCommands(uint32_t frameIndex);
  void endSingleTimeCommands(uint32_t frameIndex);
  void resetCommandBuffer(uint32_t frameIndex);

  // Record a command buffer with the provided function
  template <typename F>
  void recordCommandBuffer(uint32_t imageIndex, F&& recordFunction) {
    beginSingleTimeCommands();
    // TODO: imageIndex == frameIndex ???????
    recordFunction(m_CommandBuffers[imageIndex], imageIndex);
    endSingleTimeCommands();
  }

  // Get maximum frames in flight
  //   uint32_t getMaxFramesInFlight() const { return static_cast<uint32_t>(m_CommandBuffers.size()); }

 private:
  void createCommandPool();
  void createCommandBuffers();

 private:
  VkContext* m_Context;
  VkCommandPool m_CommandPool;

  std::vector<VkCommandBuffer> m_CommandBuffers;
  uint32_t m_maxFramesInFlight;
};

}  // namespace glint