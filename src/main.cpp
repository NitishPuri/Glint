
#include <algorithm>
#include <chrono>
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
#include "renderer/mesh.h"
#include "renderer/mesh_factory.h"
#include "renderer/pipeline.h"
#include "renderer/render_pass.h"
#include "renderer/renderer.h"
#include "renderer/swapchain.h"
#include "renderer/synchronization_manager.h"
#include "renderer/vk_context.h"
#include "samples/sample.h"
#include "samples/sample_manager.h"

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
    // initMesh();
    initSamples();

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
    props.resizable = true;

    window = std::make_unique<glint::Window>(props);
  }

  void initRenderer() {
    LOGFN;
    renderer = std::make_unique<glint::Renderer>(window.get(), MAX_FRAMES_IN_FLIGHT);
    renderer->init("./bin/shaders/shader.vert.spv", "./bin/shaders/shader.frag.spv");
  }

  // void initMesh() {
  //   LOGFN;

  //   mesh = glint::MeshFactory::createTriangle(renderer->getContext());
  //   // mesh = glint::MeshFactory::createQuad(renderer->getContext());
  // }

  void initSamples() {
    LOGFN;
    sampleManager.init(window.get(), renderer.get());

    sampleManager.registerSample(std::make_unique<glint::TriangleSample>());
    sampleManager.registerSample(std::make_unique<glint::QuadSample>());

    sampleManager.setActiveSample("Triangle Sample");
  }

  void mainLoop() {
    LOGFN;

    // For calculating delta time
    auto lastFrameTime = std::chrono::high_resolution_clock::now();

    static int frames = 0;
    LOGCALL(while (!window->shouldClose())) {
      window->pollEvents();

      // Handle sample switching
      if (window->isKeyPressed(GLFW_KEY_1)) {
        sampleManager.setActiveSample("Triangle Sample");
      } else if (window->isKeyPressed(GLFW_KEY_2)) {
        sampleManager.setActiveSample("Quad Sample");
      }

      // Calculate delta time
      auto currentTime = std::chrono::high_resolution_clock::now();
      float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastFrameTime).count();
      lastFrameTime = currentTime;

      // Update active sample
      sampleManager.update(deltaTime);

      renderer->drawFrame(
          [this](VkCommandBuffer commandBuffer, uint32_t imageIndex) { drawScene(commandBuffer, imageIndex); });

      frames++;
    }

    renderer->waitIdle();
  }

  void cleanup() {
    LOGFN;
    // cleanup will happen in correct order automatically!
    // mesh.reset();
    // renderer.reset();
    // window.reset();
  }

  std::unique_ptr<glint::Window> window = nullptr;
  std::unique_ptr<glint::Renderer> renderer = nullptr;
  // std::unique_ptr<glint::Mesh> mesh = nullptr;

  glint::SampleManager sampleManager;

  // std::unique_ptr<Sample> sample = nullptr;

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

    // LOG_ONCE("FINALLY DRAW!!!");
    // mesh->bind(commandBuffer);
    // LOGCALL_ONCE(vkCmdDraw(commandBuffer, 3, 1, 0, 0));
    // mesh->draw(commandBuffer);
    sampleManager.render(commandBuffer, imageIndex);

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