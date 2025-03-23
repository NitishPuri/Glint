#pragma once

#include "renderer/descriptor.h"
#include "renderer/ubo_data.h"
#include "sample.h"

namespace glint {

class RotatingSample : public Sample {
 public:
  RotatingSample();

  void initSample(Window* window, Renderer* renderer) override;
  void update(float deltaTime) override;
  void render(VkCommandBuffer commandBuffer, uint32_t imageIndex) override;
  void cleanup() override;

  void setMesh(std::unique_ptr<Mesh> mesh) { m_Mesh = std::move(mesh); }

 private:
  Renderer* m_Renderer = nullptr;
  std::unique_ptr<Mesh> m_Mesh = nullptr;

  // Uniform buffer resources
  std::unique_ptr<DescriptorSetLayout> m_DescriptorSetLayout;
  std::unique_ptr<DescriptorPool> m_DescriptorPool;
  std::unique_ptr<Descriptor> m_Descriptor;

  // std::unique_ptr<UniformBuffer> m_UniformBuffer;
  std::vector<std::unique_ptr<UniformBuffer>> m_UniformBuffers;

  // Transform state
  float m_RotationAngle = 0.0f;
  glm::vec3 m_RotationAxis = {0.0f, 1.0f, 0.0f};  // Rotate around Y axis

  // UBO data
  UniformBufferObject m_UBOData;
};

}  // namespace glint