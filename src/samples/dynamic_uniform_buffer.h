#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "renderer/descriptor.h"
#include "renderer/texture.h"
#include "sample.h"

namespace glint {

class DynamicUniformBuffer : public Sample {
 public:
  DynamicUniformBuffer();

  void initSample(Window* window, Renderer* renderer) override;
  void update(float deltaTime) override;
  void render(VkCommandBuffer commandBuffer, uint32_t imageIndex) override;
  void cleanup() override;

 private:
  // void updateUniformBuffers(uint32_t currentImage);

  void updateUniformBuffers();
  void updateDynamicUniformBuffer();

 private:
  void prepareUniformBuffers();
  void setupDescriptors();

  std::unique_ptr<Mesh> m_Mesh;

  // Descriptor resources
  // std::unique_ptr<DescriptorSetLayout> m_DescriptorSetLayout;
  // std::unique_ptr<DescriptorPool> m_DescriptorPool;
  // std::unique_ptr<Descriptor> m_Descriptor;
  // VkDescriptorSetLayout m_Layout = VK_NULL_HANDLE;

  // in base
  VkDescriptorPool descriptorPool{VK_NULL_HANDLE};
  // in sample
  VkDescriptorSet descriptorSet{VK_NULL_HANDLE};
  VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};

  std::vector<VkDescriptorSetLayoutBinding> m_Bindings;

  ////////

  // TODO: Sould have multiple uniform buffers for each frame in flight?
  std::unique_ptr<UniformBuffer> m_viewUBO;
  std::unique_ptr<UniformBuffer> m_dynamicUBO;

  // Transformation state
  float m_RotationAngle = 0.0f;
  glm::vec3 m_ModelPosition = glm::vec3(0.0f);
  glm::vec3 m_RotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);

  struct {
    glm::mat4 projection;
    glm::mat4 view;
  } uboVS;

  static const uint32_t OBJECT_INSTANCES = 125;
  // Store random per-object rotations
  glm::vec3 rotations[OBJECT_INSTANCES];
  glm::vec3 rotationSpeeds[OBJECT_INSTANCES];

  // One big uniform buffer that contains all matrices
  // Note that we need to manually allocate the data to cope for GPU-specific uniform buffer offset alignments
  struct UboDataDynamic {
    glm::mat4* model{nullptr};
  } uboDataDynamic;

  float animationTimer{0.0f};
  size_t dynamicAlignment{0};
};

}  // namespace glint