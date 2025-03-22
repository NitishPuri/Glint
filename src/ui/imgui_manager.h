#pragma once

#include <imgui.h>
//
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "core/window.h"
#include "logger.h"
#include "renderer/renderer.h"

namespace glint {

class ImGuiManager {
 public:
  ImGuiManager() = default;
  ~ImGuiManager();

  void init(Window* window, Renderer* renderer);
  static void newFrame();
  static void render(VkCommandBuffer commandBuffer);
  void cleanup();

 private:
  Window* window = nullptr;
  Renderer* renderer = nullptr;
  VkDescriptorPool imguiPool = VK_NULL_HANDLE;

  void createDescriptorPool();
};

}  // namespace glint