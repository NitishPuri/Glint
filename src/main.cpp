
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
#include "renderer/renderer.h"
#include "renderer/swapchain.h"
#include "renderer/synchronization_manager.h"
#include "renderer/vulkan_context.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

std::unordered_set<std::string> glint::OneTimeLogger::loggedFunctions;

const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

class App {
 public:
#pragma region APP
  void run() {
    initWindow();
    initRenderer();

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

  void initRenderer() {
    LOGFN;
    renderer = std::make_unique<glint::Renderer>(window.get(), MAX_FRAMES_IN_FLIGHT);
    renderer->init("./bin/shaders/shader.vert.spv", "./bin/shaders/shader.frag.spv");
  }

  void mainLoop() {
    LOGFN;

    static int frames = 0;
    LOGCALL(while (!window->shouldClose())) {
      window->pollEvents();

      renderer->drawFrame(
          [this](VkCommandBuffer commandBuffer, uint32_t imageIndex) { drawScene(commandBuffer, imageIndex); });

      frames++;
    }

    renderer->waitIdle();
  }

  void cleanup() {
    LOGFN;
    renderer.reset();
    window.reset();
  }

  std::unique_ptr<glint::Window> window = nullptr;
  std::unique_ptr<glint::Renderer> renderer = nullptr;

#pragma endregion APP

#pragma region SCENE

  void drawScene(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    LOGFN_ONCE;

    // Get components from renderer
    auto renderPass = renderer->getRenderPass();
    auto pipeline = renderer->getPipeline();
    auto swapChain = renderer->getSwapChain();

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPass->begin(commandBuffer, imageIndex, clearColor);
    pipeline->bind(commandBuffer);

    LOG_ONCE("Set dynamic states");
    auto swapChainExtent = swapChain->getExtent();
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    LOGCALL_ONCE(vkCmdSetScissor(commandBuffer, 0, 1, &scissor));

    LOG_ONCE("FINALLY DRAW!!!");
    LOGCALL_ONCE(vkCmdDraw(commandBuffer, 3, 1, 0, 0));

    renderPass->end(commandBuffer);
  }

#pragma endregion SCENE
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