
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

//
#include "samples/rotating_sample.h"
#include "samples/sample.h"
#include "samples/sample_manager.h"
#include "samples/textured_quad.h"

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
    renderer->init();
  }

  void initSamples() {
    LOGFN;
    sampleManager.init(window.get(), renderer.get());

    sampleManager.registerSample(std::make_unique<glint::TriangleSample>());
    sampleManager.registerSample(std::make_unique<glint::QuadSample>());
    sampleManager.registerSample(std::make_unique<glint::RotatingSample>());
    sampleManager.registerSample(std::make_unique<glint::TexturedRotatingSample>());

    // sampleManager.setActiveSample("Triangle Sample");
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
      } else if (window->isKeyPressed(GLFW_KEY_3)) {
        sampleManager.setActiveSample("RotatingSample");
      } else if (window->isKeyPressed(GLFW_KEY_4)) {
        sampleManager.setActiveSample("TexturedRotatingSample");
      } else if (window->isKeyPressed(GLFW_KEY_ESCAPE)) {
        // window->requestClose();
      }

      // Calculate delta time
      auto currentTime = std::chrono::high_resolution_clock::now();
      float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastFrameTime).count();
      lastFrameTime = currentTime;

      // Update active sample
      sampleManager.update(deltaTime);

      renderer->drawFrame([this](VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        // drawScene(commandBuffer, imageIndex);
        sampleManager.render(commandBuffer, imageIndex);
      });

      frames++;
    }

    renderer->waitIdle();
  }

  void cleanup() {
    LOGFN;
    // cleanup will happen in correct order automatically!
  }

  std::unique_ptr<glint::Window> window = nullptr;
  std::unique_ptr<glint::Renderer> renderer = nullptr;

  glint::SampleManager sampleManager;

#pragma endregion APP

#pragma region SCENE

#pragma endregion SCENE
};

#pragma region MAIN
int main() {
  LOG("Illiterate Vulkan!");
  App app;
  try {
    app.run();
  } catch (const std::exception& e) {
    LOG("[EXCEPTION]", __FUNCTION__, e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
#pragma endregion MAIN