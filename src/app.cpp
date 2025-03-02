#include <iostream>

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
  }
  ~Application() {}

  void run() {
    Scene1 scene;
    scene.onAttach();

    while (!m_Window.shouldClose()) {
      m_Window.pollEvents();

      // Start ImGui frame
      m_ImGuiLayer.beginFrame();

      // ImGui UI -> mainUI -> demo specifiic ui
      ImGui::Begin("Control Panel");
      static float color[3] = {1.0f, 1.0f, 1.0f};
      ImGui::ColorEdit3("Quad Color", color);
      ImGui::Text("Press ESC to exit.");
      ImGui::End();

      scene.onUpdate(0.0f);
      scene.onRender();

      scene.onImGuiRender();  // why seperate from onRender?

      // Render ImGui
      m_ImGuiLayer.endFrame();

      m_Window.swapBuffers();
    }
  }

 private:
  Window m_Window;
  ImGuiLayer m_ImGuiLayer;
};

int main() {
  Application app;
  app.run();
  return 0;
}
