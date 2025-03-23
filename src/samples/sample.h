#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <string>

#include "core/camera.h"
#include "renderer/mesh.h"

namespace glint {

class Window;
class Renderer;

class Sample {
 public:
  Sample(const std::string& name);
  virtual ~Sample() = default;

  // calledb by sample manager
  void init(Window* window, Renderer* renderer);

  virtual void initSample(Window* window, Renderer* renderer) = 0;
  virtual void update(float deltaTime) = 0;
  virtual void render(VkCommandBuffer commandBuffer, uint32_t imageIndex) = 0;
  virtual void cleanup() = 0;

  void setupDefaultVieportAndScissor(VkCommandBuffer commandBuffer, Renderer* renderer);
  const std::string& getName() const { return m_Name; }

  void initCamera(float aspectRatio = 1.0f, float fov = 45.0f, float nearPlane = 0.1f, float farPlane = 100.0f);
  Camera* getCamera();
  void updateCamera(float deltaTime);
  void processCameraInput();

 protected:
  std::string m_Name;

  Window* m_Window = nullptr;
  Renderer* m_Renderer = nullptr;
  std::unique_ptr<Camera> m_Camera;
};

class BasicSample : public Sample {
 public:
  BasicSample(const std::string& name = "Basic Sample");

  void initSample(Window* window, Renderer* renderer) override;
  void update(float deltaTime) override;
  void render(VkCommandBuffer commandBuffer, uint32_t imageIndex) override;
  void cleanup() override;

  void setMesh(std::unique_ptr<Mesh> mesh) { m_Mesh = std::move(mesh); }

 private:
  std::unique_ptr<Mesh> m_Mesh = nullptr;
};

class TriangleSample : public BasicSample {
 public:
  TriangleSample() : BasicSample("Triangle Sample") {}
};

class QuadSample : public BasicSample {
 public:
  QuadSample();

  void initSample(Window* window, Renderer* renderer) override;
};

}  // namespace glint