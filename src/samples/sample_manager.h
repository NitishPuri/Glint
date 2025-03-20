#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace glint {

class Sample;
class Window;
class Renderer;

class SampleManager {
 public:
  void init(Window* window, Renderer* renderer);
  void cleanup();

  void registerSample(std::unique_ptr<Sample> sample);
  void setActiveSample(const std::string& name);

  void update(float deltaTime);
  void render(VkCommandBuffer commandBuffer, uint32_t imageIndex);

 private:
  std::unordered_map<std::string, std::unique_ptr<Sample>> m_Samples;
  Sample* m_ActiveSample = nullptr;
  Window* m_Window = nullptr;
  Renderer* m_Renderer = nullptr;
};

}  // namespace glint