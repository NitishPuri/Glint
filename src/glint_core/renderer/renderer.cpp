#include "renderer.h"

#include <numeric>

#include "command_manager.h"
#include "core/logger.h"
#include "core/window.h"
#include "pipeline.h"
#include "render_pass.h"
#include "swapchain.h"
#include "synchronization_manager.h"
#include "vk_context.h"
#include "vk_utils.h"

namespace glint {

Renderer::Renderer(Window* window, uint32_t maxFramesInFlight)
    : m_Window(window), m_MaxFramesInFlight(maxFramesInFlight), m_CurrentFrame(0) {
  LOGFN;

  m_enableCommandBufferCaching = Config::isOptionSet("enable_command_buffer_caching");
}

Renderer::~Renderer() {
  LOGFN;
  // Resources will be cleaned up automatically in reverse order
}

void Renderer::init() {
  LOGFN;

  // Create Vulkan Context
  m_Context = std::make_unique<VkContext>(m_Window);
  m_Context->init();

  VkUtils::init(m_Context.get());

  // TODO: We have depeendencies here, command manager needs to exist before swap chain can be created
  //  this is because swap chain is using command pool creted inside command manager for creating and transitioning
  //  depth resources. This can be fixed by letting swap chain create and destroy its own command pool when needed.
  // Should do this in the future, after setting up some tests and benchmarks.

  // Create Command Manager
  m_CommandManager = std::make_unique<CommandManager>(m_Context.get());

  // Create SwapChain
  m_SwapChain = std::make_unique<SwapChain>(m_Context.get());
  uint32_t imageCount = m_SwapChain->getImageCount();

  // TODO: Important to map command buffers to swap chain images as we have one command buffer per swap chain image
  // previously it was being mapped to concurrent frames in, which was incorrect.
  // but it still worked because we were re-recording command buffers each frame and thus they got bound
  // to current swap chain image correctly.
  // Maybe if we want this to be more robust, we can we can mark which command buffer is bound to which swap chain image
  // and then re-record command buffers if the swap chain image is different from the current frame.
  m_CommandManager->setupCommandBuffers(imageCount);
  // m_CommandManager->setupCommandBuffers(m_MaxFramesInFlight);

  // Create RenderPass
  m_RenderPass = std::make_unique<RenderPass>(m_Context.get(), m_SwapChain.get());

  // Create Framebuffers
  m_SwapChain->createFramebuffers(m_RenderPass->getRenderPass());

  // Create Synchronization Manager
  m_SyncManager = std::make_unique<SynchronizationManager>(m_Context.get(), m_MaxFramesInFlight, imageCount);

  // Initialize images in flight
  // m_ImagesInFlight.resize(m_MaxFramesInFlight, VK_NULL_HANDLE);
  m_ImageIndices.resize(m_MaxFramesInFlight);
  std::iota(m_ImageIndices.begin(), m_ImageIndices.end(), 0);

  // command buffer tracking
  m_CommandBufferRecorded.resize(imageCount, false);
  m_CommandBuffersDirty = true;

  LOG("Renderer initialized with", m_MaxFramesInFlight, "frames in flight and", imageCount, "swap chain images");
}

void Renderer::createPipeline(const PipelineConfig* config) {
  LOGFN;

  // TODO: Have a way to cache pipelines? -> in between samples
  // TODO: Cache command buffers if needed -> in between frames
  vkDeviceWaitIdle(m_Context->getDevice());
  m_Pipeline.reset();

  m_Pipeline = std::make_unique<Pipeline>(m_Context.get(), m_RenderPass.get(), config);

  // mark command buffers dirty
  m_CommandBuffersDirty = true;
  std::fill(m_CommandBufferRecorded.begin(), m_CommandBufferRecorded.end(), false);
}

void Renderer::handleResize() {
  if (!m_Window->wasResized()) {
    return;
  }

  m_Window->waitIfMinimized();
  m_SwapChain->recreateSwapchain(m_RenderPass->getRenderPass());
  m_Window->resetResizedFlag();

  // Reset any frames in flight tracking
  //   m_ImageIndices.resize(m_MaxFramesInFlight);

  // mark command buffers dirty
  m_CommandBuffersDirty = true;
  std::fill(m_CommandBufferRecorded.begin(), m_CommandBufferRecorded.end(), false);
}

void Renderer::drawFrame(std::function<void(VkCommandBuffer, uint32_t)> recordCommandsFunc) {
  // Wait for the previous frame to finish
  // m_SyncManager->waitForFence(m_CurrentFrame);
  m_SyncManager->waitForFence(m_ImageIndices[m_CurrentFrame]);

  // Get next image from swap chain
  uint32_t imageIndex;
  VkResult result =
      vkAcquireNextImageKHR(m_Context->getDevice(), m_SwapChain->getSwapChain(), UINT64_MAX,
                            m_SyncManager->getImageAvailableSemaphore(m_CurrentFrame), VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    handleResize();
    return;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!" + __LINE__);
  }

  // Save the image index for this frame
  m_ImageIndices[m_CurrentFrame] = imageIndex;

  if (m_enableCommandBufferCaching) {
    // Check if command buffers need recording
    if (m_CommandBuffersDirty || !m_CommandBufferRecorded[imageIndex]) {
      m_CommandManager->resetCommandBuffer(imageIndex);
      m_CommandManager->beginSingleTimeCommands(imageIndex);
      recordCommandsFunc(m_CommandManager->getCommandBuffer(imageIndex), imageIndex);
      m_CommandManager->endSingleTimeCommands(imageIndex);

      // m_CommandBufferRecorded[m_CurrentFrame] = true;
      m_CommandBufferRecorded[imageIndex] = true;
      m_CommandBuffersDirty = false;
      for (auto recorded : m_CommandBufferRecorded) {
        if (!recorded) {
          m_CommandBuffersDirty = true;
          break;
        }
      }
    }
  } else {
    m_CommandManager->resetCommandBuffer(imageIndex);
    m_CommandManager->beginSingleTimeCommands(imageIndex);
    recordCommandsFunc(m_CommandManager->getCommandBuffer(imageIndex), imageIndex);
    m_CommandManager->endSingleTimeCommands(imageIndex);
  }

  // Submit command buffer
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  // Wait for image available semaphore
  VkSemaphore waitSemaphores[] = {m_SyncManager->getImageAvailableSemaphore(m_CurrentFrame)};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  // Command buffer to submit
  // VkCommandBuffer cmdBuffer = m_CommandManager->getCommandBuffer(m_CurrentFrame);
  VkCommandBuffer cmdBuffer = m_CommandManager->getCommandBuffer(imageIndex);
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cmdBuffer;

  // Signal render finished semaphore
  VkSemaphore signalSemaphores[] = {m_SyncManager->getRenderFinishedSemaphore(m_CurrentFrame)};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  // Submit to queue
  m_SyncManager->resetFence(m_ImageIndices[m_CurrentFrame]);
  VK_CHECK_RESULT(vkQueueSubmit(m_Context->getGraphicsQueue(), 1, &submitInfo,
                                m_SyncManager->getInFlightFence(m_ImageIndices[m_CurrentFrame])));

  // Present the image
  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {m_SwapChain->getSwapChain()};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.pResults = nullptr;  // Optional

  static int frameId = 0;
  result = vkQueuePresentKHR(m_Context->getPresentQueue(), &presentInfo);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Window->wasResized()) {
    handleResize();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  // Advance to the next frame
  m_CurrentFrame = (m_CurrentFrame + 1) % m_MaxFramesInFlight;
}

void Renderer::waitIdle() {
  LOGFN;
  vkDeviceWaitIdle(m_Context->getDevice());
}

}  // namespace glint