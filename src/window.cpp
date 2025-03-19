#include "window.h"

#include <iostream>

namespace glint {

static bool s_GLFWInitialized = false;

static void GLFWErrorCallback(int error, const char* description) {
  std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

Window::Window(const WindowProps& props) { init(props); }

Window::~Window() { shutdown(); }

void Window::init(const WindowProps& props) {
  m_Data.title = props.title;
  m_Data.width = props.width;
  m_Data.height = props.height;

  // Initialize GLFW if not already initialized
  if (!s_GLFWInitialized) {
    int success = glfwInit();
    if (!success) {
      throw std::runtime_error("Failed to initialize GLFW!");
    }

    glfwSetErrorCallback(GLFWErrorCallback);
    s_GLFWInitialized = true;
  }

  // Tell GLFW not to create an OpenGL context
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, props.resizable ? GLFW_TRUE : GLFW_FALSE);

  // Create the GLFW window
  m_Window = glfwCreateWindow(static_cast<int>(props.width), static_cast<int>(props.height), m_Data.title.c_str(),
                              nullptr, nullptr);

  if (!m_Window) {
    throw std::runtime_error("Failed to create GLFW window!");
  }

  // Set user pointer to access window data from callbacks
  glfwSetWindowUserPointer(m_Window, &m_Data);

  // Set up window resize callback if needed
  if (props.resizable) {
    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
      WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
      data.width = width;
      data.height = height;

      if (data.resizeCallback) {
        data.resizeCallback(width, height);
      }
    });
  }
}

void Window::shutdown() {
  glfwDestroyWindow(m_Window);

  // Only terminate GLFW if this is the last window
  // In a real application, you might want to track this globally
  glfwTerminate();
  s_GLFWInitialized = false;
}

bool Window::shouldClose() const { return glfwWindowShouldClose(m_Window); }

void Window::pollEvents() const { glfwPollEvents(); }

VkSurfaceKHR Window::createSurface(VkInstance instance) {
  VkSurfaceKHR surface;
  VkResult result = glfwCreateWindowSurface(instance, m_Window, nullptr, &surface);

  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create window surface!");
  }

  return surface;
}

std::vector<const char*> Window::getRequiredInstanceExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  return extensions;
}

}  // namespace glint