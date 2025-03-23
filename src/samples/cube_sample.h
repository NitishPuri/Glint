#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "renderer/descriptor.h"
#include "renderer/texture.h"
#include "sample.h"

namespace glint {

class CubeSample : public Sample {
 public:
  CubeSample();

  void initSample(Window* window, Renderer* renderer) override;
  void update(float deltaTime) override;
  void render(VkCommandBuffer commandBuffer, uint32_t imageIndex) override;
  void cleanup() override;

 private:
  void updateUniformBuffer(uint32_t currentImage);

 private:
  Renderer* m_Renderer = nullptr;
  std::unique_ptr<Mesh> m_Mesh;

  std::unique_ptr<Texture> m_Texture;

  // Descriptor resources
  std::unique_ptr<DescriptorSetLayout> m_DescriptorSetLayout;
  std::unique_ptr<DescriptorPool> m_DescriptorPool;
  std::unique_ptr<Descriptor> m_Descriptor;
  std::vector<std::unique_ptr<UniformBuffer>> m_UniformBuffers;

  // Transformation state
  float m_RotationAngle = 0.0f;
  glm::vec3 m_ModelPosition = glm::vec3(0.0f);
  glm::vec3 m_RotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
};

}  // namespace glint