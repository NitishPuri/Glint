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
  Sample* getActiveSample() const { return m_ActiveSample; }

  void update(float deltaTime);
  void render(VkCommandBuffer commandBuffer, uint32_t imageIndex);

 private:
  // TODO: Use create sample functions instead of unique_ptr
  std::unordered_map<std::string, std::unique_ptr<Sample>> m_Samples;
  Sample* m_ActiveSample = nullptr;
  Window* m_Window = nullptr;
  Renderer* m_Renderer = nullptr;
};

}  // namespace glint