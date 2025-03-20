#pragma once

#include <vulkan/vulkan.h>

namespace glint {

class VulkanContext;

class CommandManager {
 public:
  CommandManager(VulkanContext* context);
  ~CommandManager();

  // Prevent copying
  CommandManager(const CommandManager&) = delete;
  CommandManager& operator=(const CommandManager&) = delete;

  // Getters
  VkCommandPool getCommandPool() const { return m_CommandPool; }
  VkCommandBuffer getCommandBuffer() const { return m_CommandBuffer; }

  // Command buffer operations
  void beginSingleTimeCommands();
  void endSingleTimeCommands();
  void reset();

  // Record a command buffer with the provided function
  template <typename F>
  void recordCommandBuffer(uint32_t imageIndex, F&& recordFunction) {
    beginSingleTimeCommands();
    recordFunction(m_CommandBuffer, imageIndex);
    endSingleTimeCommands();
  }

 private:
  void createCommandPool();
  void createCommandBuffer();

 private:
  VulkanContext* m_Context;
  VkCommandPool m_CommandPool;
  VkCommandBuffer m_CommandBuffer;
};

}  // namespace glint