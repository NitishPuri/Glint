#include <chrono>
#include <iostream>
#include <memory>

#include "Core/GuiLayer.h"
#include "Core/Logger.h"
#include "Core/SceneManager.h"
#include "Core/Shader.h"
#include "Core/Window.h"

//
#include "Scenes/CubeScene.h"
#include "Scenes/QuadScene.h"

class Application {
 public:
  Application() : m_Window(800, 600, "OpenGL Window"), m_ImGuiLayer(m_Window) {}
  ~Application() {}

  void init() {
    Logger::log("Initializing application...");
    // m_CurrentScene = std::make_shared<QuadScene>();
    if (m_Window.init() == -1) {
      Logger::error("Failed to initialize window");
      exit(EXIT_FAILURE);
    }

    m_ImGuiLayer.init();

    Logger::log("Registering scenes...");
    // register scenes
    // TODO: This should be done at the place of scene definition somehow,
    // a singleton registry or something for scenes?
    m_SceneManager.registerScene("Quad", []() { return std::make_shared<QuadScene>(); });
    m_SceneManager.registerScene("Cube", []() { return std::make_shared<CubeScene>(); });
  }

  void run() {
    Logger::log("Running application...");

    auto last_time = std::chrono::high_resolution_clock::now();

    while (!m_Window.shouldClose()) {
      m_Window.pollEvents();

      auto current_time = std::chrono::high_resolution_clock::now();
      std::chrono::duration<float> delta_time = current_time - last_time;
      last_time = current_time;

      // Start ImGui frame
      m_ImGuiLayer.beginFrame();

      m_SceneManager.onUpdate(delta_time.count());
      m_SceneManager.onRender();
      m_SceneManager.onImGuiRender();

      // Render ImGui
      m_ImGuiLayer.endFrame();

      m_Window.swapBuffers();
    }
  }

 private:
  Window m_Window;
  ImGuiLayer m_ImGuiLayer;
  SceneManager m_SceneManager;
};

int main() {
  Logger::init("./log.txt");
  Logger::log("Starting application");
  Application app;
  app.init();

  app.run();

  return 0;
}
