#include "renderer.h"

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

  // Create Command Manager
  m_CommandManager = std::make_unique<CommandManager>(m_Context.get(), m_MaxFramesInFlight);
  m_Context->setCommandPool(m_CommandManager->getCommandPool());  // TODO: Remove this

  // Create SwapChain
  m_SwapChain = std::make_unique<SwapChain>(m_Context.get());

  // Create RenderPass
  m_RenderPass = std::make_unique<RenderPass>(m_Context.get(), m_SwapChain.get());

  // Create Framebuffers
  m_SwapChain->createFramebuffers(m_RenderPass->getRenderPass());

  // Create Synchronization Manager
  m_SyncManager = std::make_unique<SynchronizationManager>(m_Context.get(), m_MaxFramesInFlight);

  // Initialize images in flight
  uint32_t imageCount = m_SwapChain->getImageCount();
  //   m_ImagesInFlight.resize(imageCount, VK_NULL_HANDLE);
  m_ImageIndices.resize(m_MaxFramesInFlight);

  LOG("Renderer initialized with", m_MaxFramesInFlight, "frames in flight and", imageCount, "swap chain images");
}

void Renderer::createPipeline(const PipelineConfig* config) {
  LOGFN;

  // TODO: Have a way to cache pipelines? -> in between samples
  // TODO: Cache command buffers if needed -> in between frames
  vkDeviceWaitIdle(m_Context->getDevice());
  m_Pipeline.reset();

  m_Pipeline = std::make_unique<Pipeline>(m_Context.get(), m_RenderPass.get(), config);
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
}

void Renderer::drawFrame(std::function<void(VkCommandBuffer, uint32_t)> recordCommandsFunc) {
  LOGFN_ONCE;

  LOG_ONCE("--------------------------------------------------------------");
  LOG_ONCE("Outline of a frame..");
  LOG_ONCE("Wait for the previous frame to be finished");
  LOG_ONCE("Acquire an image from the swap chain");
  LOG_ONCE("Record a command buffer which draws the scene onto the image.");
  LOG_ONCE("Submit the command buffer to the graphics queue.");
  LOG_ONCE("Present the image to the swap chain for presentation.");
  LOG_ONCE("--------------------------------------------------------------");

  // Wait for the previous frame to finish
  m_SyncManager->waitForFence(m_CurrentFrame);

  // Get next image from swap chain
  uint32_t imageIndex;
  VkResult result =
      vkAcquireNextImageKHR(m_Context->getDevice(), m_SwapChain->getSwapChain(), UINT64_MAX,
                            m_SyncManager->getImageAvailableSemaphore(m_CurrentFrame), VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    handleResize();
    return;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to acquire swap chain image!");
  }

  // Save the image index for this frame
  m_ImageIndices[m_CurrentFrame] = imageIndex;

  // Reset and record command buffer
  m_CommandManager->resetCommandBuffer(m_CurrentFrame);

  // m_CommandManager->recordCommandBuffer(m_CurrentFrame, recordCommandsFunc);

  m_CommandManager->beginSingleTimeCommands(m_CurrentFrame);
  recordCommandsFunc(m_CommandManager->getCommandBuffer(m_CurrentFrame), imageIndex);
  m_CommandManager->endSingleTimeCommands(m_CurrentFrame);

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
  VkCommandBuffer cmdBuffer = m_CommandManager->getCommandBuffer(m_CurrentFrame);
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cmdBuffer;

  // Signal render finished semaphore
  VkSemaphore signalSemaphores[] = {m_SyncManager->getRenderFinishedSemaphore(m_CurrentFrame)};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  // Submit to queue
  m_SyncManager->resetFence(m_CurrentFrame);
  if (vkQueueSubmit(m_Context->getGraphicsQueue(), 1, &submitInfo, m_SyncManager->getInFlightFence(m_CurrentFrame)) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to submit draw command buffer!");
  }

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

  LOGCALL_ONCE(result = vkQueuePresentKHR(m_Context->getPresentQueue(), &presentInfo));
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