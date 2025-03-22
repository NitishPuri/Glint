
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

#include "core/config.h"
#include "core/logger.h"
#include "core/window.h"
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
#include "cube_sample.h"
#include "imgui_manager.h"
#include "rotating_sample.h"
#include "sample.h"
#include "sample_manager.h"
#include "textured_quad.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

std::unordered_set<std::string> glint::OneTimeLogger::loggedFunctions;

const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

class App {
 public:
  void run() {
    initWindow();
    initRenderer();
    initImgui();
    initSamples();

    mainLoop();
    cleanup();
  }

 private:
  void initWindow() {
    LOGFN;
    glint::Window::WindowProps props;
    props.title = "Glint - Samples";
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

  void initImgui() {
    LOGFN;
    imguiManager = std::make_unique<glint::ImGuiManager>();
    imguiManager->init(window.get(), renderer.get());
  }

  void initSamples() {
    LOGFN;
    sampleManager.init(window.get(), renderer.get());

    sampleManager.registerSample(std::make_unique<glint::TriangleSample>());
    sampleManager.registerSample(std::make_unique<glint::QuadSample>());
    sampleManager.registerSample(std::make_unique<glint::RotatingSample>());
    sampleManager.registerSample(std::make_unique<glint::TexturedRotatingSample>());
    sampleManager.registerSample(std::make_unique<glint::CubeSample>());

    sampleManager.setActiveSample("CubeSample");
  }

  void mainLoop() {
    LOGFN;

    // For calculating delta time
    auto lastFrameTime = std::chrono::high_resolution_clock::now();
    float fps = 0.0f;
    float frameTime = 0.0f;

    static int frames = 0;
    while (!(window->shouldClose() || window->isKeyPressed(GLFW_KEY_ESCAPE))) {
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
      } else if (window->isKeyPressed(GLFW_KEY_5)) {
        sampleManager.setActiveSample("CubeSample");
      }

      // Calculate delta time
      auto currentTime = std::chrono::high_resolution_clock::now();
      float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastFrameTime).count();
      lastFrameTime = currentTime;

      // Update FPS
      frameTime = deltaTime;
      fps = 1.0f / deltaTime;

      // Update active sample
      sampleManager.update(deltaTime);

      glint::ImGuiManager::newFrame();

      ImGui::Begin("Sample Selector");
      if (ImGui::Button("Triangle")) {
        sampleManager.setActiveSample("Triangle Sample");
      }
      if (ImGui::Button("Quad")) {
        sampleManager.setActiveSample("Quad Sample");
      }
      if (ImGui::Button("Rotating")) {
        sampleManager.setActiveSample("RotatingSample");
      }
      if (ImGui::Button("Textured Rotating")) {
        sampleManager.setActiveSample("TexturedRotatingSample");
      }
      if (ImGui::Button("Cube")) {
        sampleManager.setActiveSample("CubeSample");
      }
      ImGui::End();

      ImGui::Begin("Glint Stats");
      ImGui::Text("FPS: %.1f", fps);
      ImGui::Text("Frame Time: %.3f ms", frameTime * 1000.0f);
      ImGui::Text("Active Sample: %s", sampleManager.getActiveSample()->getName().c_str());
      ImGui::End();

      renderer->drawFrame([this](VkCommandBuffer commandBuffer, uint32_t imageIndex) {
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

  std::unique_ptr<glint::ImGuiManager> imguiManager = nullptr;

  glint::SampleManager sampleManager;
};

int main(int argc, char** argv) {
  glint::Config::initialize(argc, argv);

  LOG("Glint Sample Browser!");
  App app;
  try {
    app.run();
  } catch (const std::exception& e) {
    LOG("[EXCEPTION]", __FUNCTION__, e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
