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
    std::string title;
    uint32_t width;
    uint32_t height;
    bool resizable;

    WindowProps(const std::string& title = "Glint Vulkan", uint32_t width = 800, uint32_t height = 600,
                bool resizable = false)
        : title(title), width(width), height(height), resizable(resizable) {}
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

 private:
  void init(const WindowProps& props);
  void shutdown();

 private:
  GLFWwindow* m_Window;

  struct WindowData {
    std::string title;
    uint32_t width, height;
    bool vsync;

    // For window resize callback
    std::function<void(uint32_t, uint32_t)> resizeCallback;
  };

  WindowData m_Data;
};

}  // namespace glint