// Window.h
#pragma once
#include "glad/glad.h"
//
#include <GLFW/glfw3.h>

#include <iostream>

class Window {
 public:
  Window(int width, int height, const std::string& title)
      : m_Window(nullptr), m_Width(width), m_Height(height), m_Title(title) {}
  ~Window() {
    glfwDestroyWindow(m_Window);
    glfwTerminate();
  }
  GLFWwindow* getGLFWWindow() const { return m_Window; }

  int init() {
    if (!glfwInit()) {
      std::cerr << "Failed to initialize GLFW\n";
      exit(EXIT_FAILURE);
      return -1;
    }

    m_Window =
        glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
    if (!m_Window) {
      std::cerr << "Failed to create GLFW window\n";
      glfwTerminate();
      exit(EXIT_FAILURE);
      return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(m_Window, GLFW_STICKY_KEYS, GL_TRUE);

    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1);  // Enable vsync
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    return 0;
  }

  void pollEvents() {
    glfwPollEvents();
    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
    }
  }
  bool shouldClose() const { return glfwWindowShouldClose(m_Window); }
  void swapBuffers() { glfwSwapBuffers(m_Window); }

 private:
  GLFWwindow* m_Window;
  int m_Width, m_Height;
  std::string m_Title;
};
