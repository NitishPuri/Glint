#include <iostream>
#include <memory>

#include "Core/GuiLayer.h"
#include "Core/Shader.h"
#include "Core/Window.h"
#include "Scenes/BaseScene.h"

class Application {
 public:
  Application() : m_Window(800, 600, "OpenGL Window"), m_ImGuiLayer(m_Window) {
    if (m_Window.init() == -1) {
      std::cerr << "Failed to initialize window\n";
      exit(EXIT_FAILURE);
    }

    m_ImGuiLayer.init();

    m_CurrentScene = std::make_shared<QuadScene>();
  }
  ~Application() {}

  void run() {
    // QuadScene scene;
    m_CurrentScene->onAttach();

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

      m_CurrentScene->onUpdate(0.0f);
      m_CurrentScene->onRender();
      m_CurrentScene->onImGuiRender();  // why seperate from onRender?

      // Render ImGui
      m_ImGuiLayer.endFrame();

      m_Window.swapBuffers();
    }
  }

 private:
  Window m_Window;
  ImGuiLayer m_ImGuiLayer;
  std::shared_ptr<SceneBase> m_CurrentScene;
};

int main() {
  Application app;
  app.run();
  return 0;
}
