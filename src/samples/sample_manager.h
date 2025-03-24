#pragma once

#include <vulkan/vulkan.h>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace glint {

class Sample;
class Window;
class Renderer;

class SampleManager {
 public:
  static SampleManager& getInstance() {
    static SampleManager instance;
    return instance;
  }

  template <typename T>
  static bool registerSample(const std::string& name) {
    SampleManager& instance = getInstance();
    instance.registerSample(name, []() { return std::make_unique<T>(); });
    return true;
  }

  static std::unique_ptr<Sample> createSample(const std::string& name);

  static std::vector<std::string> getSampleNames();

  static void init(Window* window, Renderer* renderer);
  static void cleanup();

  // void registerSample(std::unique_ptr<Sample> sample);
  void setActiveSample(const std::string& name);
  static Sample* getActiveSample() { return getInstance().m_ActiveSample.get(); }

  static void update(float deltaTime);
  static void render(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    getInstance().renderSample(commandBuffer, imageIndex);
  }

 private:
  SampleManager();

  void registerSample(const std::string& name, std::function<std::unique_ptr<Sample>()> createFn);
  void renderSample(VkCommandBuffer commandBuffer, uint32_t imageIndex);

  std::unordered_map<std::string, std::function<std::unique_ptr<Sample>()>> m_SampleCreators;
  std::vector<std::string> m_SampleNames;  // for ordering

  std::unique_ptr<Sample> m_ActiveSample;
  Window* m_Window;
  Renderer* m_Renderer;
};

}  // namespace glint

#define REGISTER_SAMPLE(SampleClass)                                                               \
  namespace {                                                                                      \
  bool SampleClass##_registered = glint::SampleManager::registerSample<SampleClass>(#SampleClass); \
  }