#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <string>

#include "renderer/mesh.h"

namespace glint {

class Window;
class Renderer;

class Sample {
 public:
  Sample(const std::string& name);
  virtual ~Sample() = default;

  virtual void init(Window* window, Renderer* renderer) = 0;
  virtual void update(float deltaTime) = 0;
  virtual void render(VkCommandBuffer commandBuffer, uint32_t imageIndex) = 0;
  virtual void cleanup() = 0;

  void setupDefaultVieportAndScissor(VkCommandBuffer commandBuffer, Renderer* renderer);
  const std::string& getName() const { return m_Name; }

 protected:
  std::string m_Name;
};

class BasicSample : public Sample {
 public:
  BasicSample(const std::string& name = "Basic Sample");

  void init(Window* window, Renderer* renderer) override;
  void update(float deltaTime) override;
  void render(VkCommandBuffer commandBuffer, uint32_t imageIndex) override;
  void cleanup() override;

  void setMesh(std::unique_ptr<Mesh> mesh) { m_Mesh = std::move(mesh); }

 private:
  Renderer* m_Renderer = nullptr;
  std::unique_ptr<Mesh> m_Mesh = nullptr;
};

class TriangleSample : public BasicSample {
 public:
  TriangleSample() : BasicSample("Triangle Sample") {}
};

class QuadSample : public BasicSample {
 public:
  QuadSample();

  void init(Window* window, Renderer* renderer) override;
};

// class CubeSample : public Sample {
//  public:
//   CubeSample();

//   void init(Window* window, Renderer* renderer) override;
//   void update(float deltaTime) override;
//   void render(VkCommandBuffer commandBuffer, uint32_t imageIndex) override;
//   void cleanup() override;

//  private:
//   Renderer* m_Renderer = nullptr;
//   std::unique_ptr<Camera> m_Camera;
//   std::unique_ptr<Mesh> m_CubeMesh;
//   std::unique_ptr<Material> m_Material;
//   std::unique_ptr<UniformBuffer> m_UniformBuffer;
//   float m_Rotation = 0.0f;
// };

}  // namespace glint