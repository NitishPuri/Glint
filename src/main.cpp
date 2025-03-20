
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

#include "core/window.h"
#include "logger.h"
#include "renderer/command_manager.h"
#include "renderer/pipeline.h"
#include "renderer/render_pass.h"
#include "renderer/swapchain.h"
#include "renderer/synchronization_manager.h"
#include "renderer/vulkan_context.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

std::unordered_set<std::string> glint::OneTimeLogger::loggedFunctions;

class App {
 public:
#pragma region APP
  void run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
  }

 private:
  void initWindow() {
    LOGFN;
    glint::Window::WindowProps props;
    props.title = "Vulkan";
    props.width = WIDTH;
    props.height = HEIGHT;
    props.resizable = false;

    window = std::make_unique<glint::Window>(props);
  }

  void initVulkan() {
    LOGFN;

    // Create Vulkan Context
    vulkanContext = std::make_unique<glint::VulkanContext>(window.get());
    vulkanContext->init();

    // Get device from context
    device = vulkanContext->getDevice();
    graphicsQueue = vulkanContext->getGraphicsQueue();
    presentQueue = vulkanContext->getPresentQueue();

    // Create SwapChain
    swapChain = std::make_unique<glint::SwapChain>(vulkanContext.get());
    swapChainExtent = swapChain->getExtent();

    // Create RenderPass
    renderPass = std::make_unique<glint::RenderPass>(vulkanContext.get(), swapChain.get());

    const std::string vertShaderPath = "./bin/shaders/shader.vert.spv";
    const std::string fragShaderPath = "./bin/shaders/shader.frag.spv";
    pipeline = std::make_unique<glint::Pipeline>(vulkanContext.get(), renderPass.get(), vertShaderPath, fragShaderPath);

    // Create Framebuffers
    swapChain->createFramebuffers(renderPass->getRenderPass());

    commandManager = std::make_unique<glint::CommandManager>(vulkanContext.get());

    createSyncObjects();
  }

  void mainLoop() {
    LOGFN;

    static int frames = 0;
    // LOGCALL(while (!glfwWindowShouldClose(window))) {
    LOGCALL(while (!window->shouldClose())) {
      glfwPollEvents();
      drawFrame();

      frames++;
      // if (frames == 2) break;
    }

    LOGCALL(vkDeviceWaitIdle(device));
  }

  void cleanup() {
    LOGFN;

    LOGCALL(vkDestroySemaphore(device, renderFinishedSemaphore, nullptr));
    LOGCALL(vkDestroySemaphore(device, imageAvailableSemaphore, nullptr));
    LOGCALL(vkDestroyFence(device, inFlightFence, nullptr));

    commandManager.reset();
    pipeline.reset();
    renderPass.reset();
    swapChain.reset();
    vulkanContext.reset();
    window.reset();
  }

#pragma endregion APP

#pragma region VARIABLES
  std::unique_ptr<glint::Window> window = nullptr;
  std::unique_ptr<glint::VulkanContext> vulkanContext = nullptr;
  std::unique_ptr<glint::SwapChain> swapChain = nullptr;
  std::unique_ptr<glint::RenderPass> renderPass = nullptr;
  std::unique_ptr<glint::Pipeline> pipeline = nullptr;
  std::unique_ptr<glint::CommandManager> commandManager = nullptr;

  VkDevice device;
  VkQueue graphicsQueue;

  VkQueue presentQueue;

  VkExtent2D swapChainExtent;

  // VkCommandPool commandPool;
  // VkCommandBuffer commandBuffer;

  VkSemaphore imageAvailableSemaphore;
  VkSemaphore renderFinishedSemaphore;
  VkFence inFlightFence;

#pragma endregion VARIABLES

#pragma region COMMAND_BUFFERS

  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    LOGFN_ONCE;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                   // Optional
    beginInfo.pInheritanceInfo = nullptr;  // Optional

    if (LOGCALL_ONCE(vkBeginCommandBuffer(commandBuffer, &beginInfo)) != VK_SUCCESS) {
      throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPass->begin(commandBuffer, imageIndex, clearColor);

    LOG_ONCE("Bind Pipeline");
    LOGCALL_ONCE(vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipeline()));

    LOG_ONCE("Set dynamic states");
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    LOGCALL_ONCE(vkCmdSetViewport(commandBuffer, 0, 1, &viewport));

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    LOGCALL_ONCE(vkCmdSetScissor(commandBuffer, 0, 1, &scissor));

    LOG_ONCE("FINALLY DRAW!!!");
    LOGCALL_ONCE(vkCmdDraw(commandBuffer, 3, 1, 0, 0));

    renderPass->end(commandBuffer);

    if (LOGCALL_ONCE(vkEndCommandBuffer(commandBuffer)) != VK_SUCCESS) {
      throw std::runtime_error("failed to record command buffer!");
    }

    LOG_ONCE("Command Buffer Recorded");
  }

#pragma endregion COMMAND_BUFFERS

#pragma region SYNCHRONISATION
  void createSyncObjects() {
    LOGFN;
    LOG("Synchronization:");
    LOG("Semaphors: signal and wait for the image available and render finished, sync between queues");
    LOG("Fences: wait for the frame to finish before starting the next one, sync between CPU and GPU");

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    LOG("Create fence in signaled state, so that the first frame can start immediately");
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    LOG("vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore)");
    LOG("vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore)");
    LOG("vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence)");
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
      throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
  }
#pragma endregion SYNCHRONISATION

#pragma region DRAW_FRAMES
  void drawFrame() {
    LOGFN_ONCE;
    LOG_ONCE("--------------------------------------------------------------");
    LOG_ONCE("Outline of a frame..");
    LOG_ONCE("Wait for the previous frame to be finished");
    LOG_ONCE("Acquire an image from the swap chain");
    LOG_ONCE("Record a command buffer which draws the scene onto the image.");
    LOG_ONCE("Submit the command buffer to the graphics queue.");
    LOG_ONCE("Present the image to the swap chain for presentation.");
    LOG_ONCE("--------------------------------------------------------------");

    LOG_ONCE("Wait for the previous frame to be finished");
    LOGCALL_ONCE(vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX));
    LOGCALL_ONCE(vkResetFences(device, 1, &inFlightFence));

    LOG_ONCE("Acquire an image from the swap chain");
    uint32_t imageIndex;
    LOGCALL_ONCE(vkAcquireNextImageKHR(device, swapChain->getSwapChain(), UINT64_MAX, imageAvailableSemaphore,
                                       VK_NULL_HANDLE, &imageIndex));

    auto commandBuffer = commandManager->getCommandBuffer();
    LOGCALL_ONCE(vkResetCommandBuffer(commandBuffer, 0));
    LOG_ONCE("Record a command buffer which draws the scene onto the image.");
    recordCommandBuffer(commandBuffer, imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    LOG_ONCE("Wait for the imageAvailableSemaphore..");
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    LOG_ONCE("Wait till the color attachment is ready for writing..");
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    LOG_ONCE("Submit the command buffer to the graphics queue");
    if (LOGCALL_ONCE(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence)) != VK_SUCCESS) {
      throw std::runtime_error("failed to submit draw command buffer!");
    }

    // Presentation
    LOG_ONCE("Presentation");
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    LOG_ONCE("Wait for the renderFinishedSemaphore..");
    presentInfo.pWaitSemaphores = signalSemaphores;

    LOG_ONCE("Specify swap chain to present to.");
    VkSwapchainKHR swapChains[] = {swapChain->getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;  // Optional

    LOG_ONCE("Present the image to the swap chain for presentation.");
    if (LOGCALL_ONCE(vkQueuePresentKHR(presentQueue, &presentInfo)) != VK_SUCCESS) {
      throw std::runtime_error("failed to present swap chain image!");
    }

    // firstFrame = false;
  }
#pragma endregion DRAW_FRAMES
};

#pragma region MAIN
int main() {
  LOG("Illiterate Vulkan!");
  App app;
  try {
    app.run();
  } catch (const std::exception& e) {
    LOG("[ERROR]", __FUNCTION__, e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
#pragma endregion MAIN