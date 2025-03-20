#pragma once

#include <vulkan/vulkan.h>

#include <functional>
#include <memory>
#include <string>

namespace glint {

class Window;
class VulkanContext;
class SwapChain;
class RenderPass;
class Pipeline;
class CommandManager;
class SynchronizationManager;

class Renderer {
 public:
  Renderer(Window* window, uint32_t maxFramesInFlight);
  ~Renderer();

  // Prevent copying
  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;

  void init(const std::string& vertShaderPath, const std::string& fragShaderPath);
  void drawFrame(std::function<void(VkCommandBuffer, uint32_t)> recordCommandsFunc);
  void waitIdle();

  // Getters
  RenderPass* getRenderPass() const { return m_RenderPass.get(); }
  Pipeline* getPipeline() const { return m_Pipeline.get(); }
  SwapChain* getSwapChain() const { return m_SwapChain.get(); }

  void handleResize();

 private:
  Window* m_Window;
  std::unique_ptr<VulkanContext> m_Context;
  std::unique_ptr<SwapChain> m_SwapChain;
  std::unique_ptr<RenderPass> m_RenderPass;
  std::unique_ptr<Pipeline> m_Pipeline;
  std::unique_ptr<CommandManager> m_CommandManager;
  std::unique_ptr<SynchronizationManager> m_SyncManager;

  // Keep track of images in flight
  uint32_t m_MaxFramesInFlight;
  uint32_t m_CurrentFrame = 0;

  //   std::vector<VkFence> m_ImagesInFlight;
  std::vector<uint32_t> m_ImageIndices;
};

}  // namespace glint