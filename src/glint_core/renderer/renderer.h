#pragma once

#include <vulkan/vulkan.h>

#include <functional>
#include <memory>
#include <string>

namespace glint {

class Window;
class VkContext;
class SwapChain;
class RenderPass;
class Pipeline;
struct PipelineConfig;
class CommandManager;
class SynchronizationManager;
class DescriptorSetLayout;

class Renderer {
 public:
  Renderer(Window* window, uint32_t maxFramesInFlight);
  ~Renderer();

  // Prevent copying
  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;

  void init();
  void drawFrame(std::function<void(VkCommandBuffer, uint32_t)> recordCommandsFunc);
  void waitIdle();

  // Create pipeline with descriptor set layout
  void createPipeline(const PipelineConfig* config = nullptr);

  // Getters
  VkContext* getContext() const { return m_Context.get(); }
  RenderPass* getRenderPass() const { return m_RenderPass.get(); }
  Pipeline* getPipeline() const { return m_Pipeline.get(); }
  SwapChain* getSwapChain() const { return m_SwapChain.get(); }

  uint32_t getFramesInFlight() const { return m_MaxFramesInFlight; }
  uint32_t getCurrentFrame() const { return m_CurrentFrame; }

  void handleResize();

  void markCommandBuffersDirty() {
    m_CommandBuffersDirty = true;
    std::fill(m_CommandBufferRecorded.begin(), m_CommandBufferRecorded.end(), false);
  }

 private:
  Window* m_Window;
  std::unique_ptr<VkContext> m_Context;
  std::unique_ptr<SwapChain> m_SwapChain;
  std::unique_ptr<RenderPass> m_RenderPass;
  std::unique_ptr<Pipeline> m_Pipeline;
  std::unique_ptr<CommandManager> m_CommandManager;
  std::unique_ptr<SynchronizationManager> m_SyncManager;

  DescriptorSetLayout* m_DescriptorSetLayout = nullptr;

  // Keep track of images in flight
  uint32_t m_MaxFramesInFlight;
  uint32_t m_CurrentFrame = 0;

  // std::vector<VkFence> m_ImagesInFlight;
  std::vector<uint32_t> m_ImageIndices;

  // maybe move this in Cmmand manager ?
  bool m_CommandBuffersDirty = true;
  std::vector<bool> m_CommandBufferRecorded;
};

}  // namespace glint