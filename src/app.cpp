#include <iostream>
#include <memory>

#include "Core/GuiLayer.h"
#include "Core/Logger.h"
#include "Core/SceneManager.h"
#include "Core/Shader.h"
#include "Core/Window.h"

//
#include "Scenes/BaseScene.h"

class Application {
 public:
  Application() : m_Window(800, 600, "OpenGL Window"), m_ImGuiLayer(m_Window) {}
  ~Application() {}

  void init() {
    Logger::log("Initializing application");
    // m_CurrentScene = std::make_shared<QuadScene>();
    if (m_Window.init() == -1) {
      Logger::error("Failed to initialize window");
      exit(EXIT_FAILURE);
    }

    m_ImGuiLayer.init();

    // register scenes
    // TODO: This should be done at the place of scene definition somehow,
    // a singleton registry or something for scenes?
    m_SceneManager.registerScene("Quad", []() { return std::make_shared<QuadScene>(); });
  }

  void run() {
    // QuadScene scene;
    // m_CurrentScene->onAttach();

    while (!m_Window.shouldClose()) {
      m_Window.pollEvents();

      // Start ImGui frame
      m_ImGuiLayer.beginFrame();

      // ImGui UI -> mainUI -> demo specifiic ui
      ImGui::Begin("Scene Control Panel");
      static float color[3] = {1.0f, 1.0f, 1.0f};
      ImGui::ColorEdit3("Quad Color", color);
      ImGui::Text("Press ESC to exit.");
      ImGui::End();

      m_SceneManager.onUpdate(0.0f);
      m_SceneManager.onRender();
      m_SceneManager.onImGuiRender();

      // m_CurrentScene->onUpdate(0.0f);
      // m_CurrentScene->onRender();
      // m_CurrentScene->onImGuiRender();  // why seperate from onRender?

      // Render ImGui
      m_ImGuiLayer.endFrame();

      m_Window.swapBuffers();
    }
  }

 private:
  Window m_Window;
  ImGuiLayer m_ImGuiLayer;
  SceneManager m_SceneManager;
  // const Logger& m_Logger;
  // std::shared_ptr<SceneBase> m_CurrentScene;
};

int main() {
  Logger::init("./log.txt");
  Logger::log("Starting application");
  Application app;
  app.init();
  // if (app.init() == -1) {
  //   logger.log("Failed to initialize application");
  //   return -1;
  // }
  app.run();
  return 0;
}
