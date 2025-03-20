#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <functional>
#include <stdexcept>
#include <string>

namespace glint {

class Window {
 public:
  struct WindowProps {
    std::string title = "Glint";
    uint32_t width = 800;
    uint32_t height = 600;
    bool resizable = true;
  };

  Window(const WindowProps& props = WindowProps());
  ~Window();

  // Prevent copying
  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  bool shouldClose() const;
  void pollEvents() const;

  // Getters
  uint32_t getWidth() const { return m_Data.width; }
  uint32_t getHeight() const { return m_Data.height; }
  GLFWwindow* getNativeWindow() const { return m_Window; }

  // Create Vulkan surface
  VkSurfaceKHR createSurface(VkInstance instance);

  // Get required instance extensions
  static std::vector<const char*> getRequiredInstanceExtensions();

  // Set window resize callback
  void setResizeCallback(const std::function<void(uint32_t, uint32_t)>& callback) { m_Data.resizeCallback = callback; }

  bool wasResized() const { return m_Data.resized; }
  void resetResizedFlag() { m_Data.resized = false; }
  //  GLFWwindow* getGLFWWindow() const { return m_Window; }

  void waitIfMinimized() {
    int width = 0, height = 0;
    while (width == 0 || height == 0) {
      glfwGetFramebufferSize(m_Window, &width, &height);
      glfwWaitEvents();
    }
  }

 private:
  void init(const WindowProps& props);
  void shutdown();

  // GLFW callback function wrappers
  // static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

 private:
  GLFWwindow* m_Window;

  struct WindowData {
    std::string title;
    uint32_t width, height;
    bool resized = false;
    // bool vsync;

    // For window resize callback
    std::function<void(uint32_t, uint32_t)> resizeCallback;
  };

  WindowData m_Data;
};

}  // namespace glint