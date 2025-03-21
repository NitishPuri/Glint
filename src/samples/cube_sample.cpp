#include "cube_sample.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "logger.h"
#include "renderer/descriptor.h"
#include "renderer/mesh_factory.h"
#include "renderer/pipeline.h"
#include "renderer/render_pass.h"
#include "renderer/renderer.h"
#include "renderer/swapchain.h"
#include "renderer/texture.h"
#include "renderer/ubo_data.h"
#include "renderer/vk_utils.h"

namespace glint {

CubeSample::CubeSample() : Sample("CubeSample") { LOGFN; }

void CubeSample::init(Window* window, Renderer* renderer) {
  LOGFN;
  m_Renderer = renderer;

  uint32_t framesInFlight = renderer->getFramesInFlight();
  LOG("Creating resources for", framesInFlight, "frames in flight");

  // Create a textured quad mesh
  // m_Mesh = MeshFactory::createCube(renderer->getContext());
  m_Mesh = MeshFactory::createTexturedCube(renderer->getContext());

  // Load texture
  m_Texture = std::make_unique<Texture>(renderer->getContext(), "./res/texture.jpg");

  // Create descriptor set layout
  m_DescriptorSetLayout = DescriptorSetLayout::Builder(renderer->getContext())
                              .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                              .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                              .build();

  // Create pipeline with textured shader
  PipelineConfig config;
  config.descriptorSetLayout = m_DescriptorSetLayout.get();
  // config.vertexShaderPath = getShaderPath("shader.vert");
  // config.fragmentShaderPath = getShaderPath("shader.frag");
  // config.vertexFormat = VertexAttributeFlags::POSITION_COLOR;
  config.vertexShaderPath = getShaderPath("basic_tex.vert");
  config.fragmentShaderPath = getShaderPath("basic_tex.frag");
  config.vertexFormat = VertexAttributeFlags::POSITION_COLOR_TEXCOORD;
  config.depthTestEnable = true;
  config.depthWriteEnable = true;

  config.cullMode = VK_CULL_MODE_BACK_BIT;
  //   config.blendEnable = true;

  renderer->createPipeline(&config);

  // Create descriptor pool with enough capacity for uniform buffers and
  // textures
  m_DescriptorPool =
      std::make_unique<DescriptorPool>(renderer->getContext(), m_DescriptorSetLayout.get(), framesInFlight);

  // Create uniform buffer for each frame in flight
  m_UniformBuffers.resize(framesInFlight);
  for (auto& ubo : m_UniformBuffers) {
    ubo = std::make_unique<UniformBuffer>(renderer->getContext(), sizeof(UniformBufferObject));
  }

  // Create descriptor
  m_Descriptor = std::make_unique<Descriptor>(renderer->getContext(), m_DescriptorSetLayout.get(),
                                              m_DescriptorPool.get(), framesInFlight);

  // Update descriptor with uniform buffer and texture
  for (uint32_t i = 0; i < framesInFlight; i++) {
    // Update uniform buffer
    m_Descriptor->updateUniformBuffer(m_UniformBuffers[i]->getBuffer(), sizeof(UniformBufferObject), 0, i);

    // Update texture sampler
    m_Descriptor->updateTextureSampler(m_Texture->getImageView(), m_Texture->getSampler(), i);
  }

  // Set initial transformation matrices
  VkExtent2D extent = renderer->getSwapChain()->getExtent();
  float aspect = extent.width / (float)extent.height;

  // Initial transformations will be set in updateUniformBuffer
}

void CubeSample::update(float deltaTime) {
  // Update rotation angle
  m_RotationAngle += deltaTime * 45.0f;  // 45 degrees per second

  // Keep angle between 0-360 degrees
  if (m_RotationAngle > 360.0f) {
    m_RotationAngle -= 360.0f;
  }

  static auto t = deltaTime;
  t += deltaTime;
  auto f = std::sin(t);
  m_ModelPosition = glm::vec3(f / 2, 0.0f, 0.0f);
  m_RotationAxis = glm::mix(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), f);

  // Get current frame index
  uint32_t currentFrame = m_Renderer->getCurrentFrame();

  // Update the uniform buffer with new transformation
  updateUniformBuffer(currentFrame);
}

void CubeSample::updateUniformBuffer(uint32_t currentImage) {
  UniformBufferObject ubo{};

  // Create model matrix with rotation around z-axis
  ubo.model = glm::translate(glm::mat4(1.0f), m_ModelPosition);
  ubo.model = glm::rotate(ubo.model, glm::radians(m_RotationAngle), m_RotationAxis);

  // View matrix - slight distance from the quad
  ubo.view = glm::lookAt(glm::vec3(2.0f, 0.2f, 2.0f),   // Camera position
                         glm::vec3(0.0f, 0.0f, 0.0f),   // Look at origin
                         glm::vec3(0.0f, 1.0f, 0.0f));  // Up vector

  // Projection matrix
  VkExtent2D extent = m_Renderer->getSwapChain()->getExtent();
  float aspect = extent.width / (float)extent.height;
  ubo.proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

  // Flip Y coordinate for Vulkan
  ubo.proj[1][1] *= -1;

  // Update the uniform buffer
  m_UniformBuffers[currentImage]->update(&ubo, sizeof(ubo));
}

void CubeSample::render(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
  // Begin render pass
  auto renderPass = m_Renderer->getRenderPass();
  auto pipeline = m_Renderer->getPipeline();
  uint32_t currentFrame = m_Renderer->getCurrentFrame();

  renderPass->begin(commandBuffer, imageIndex, {0.0f, 0.1f, 0.2f, 1.0f});

  // Bind pipeline and descriptor sets
  pipeline->bind(commandBuffer);
  m_Descriptor->bind(commandBuffer, pipeline->getPipelineLayout(), currentFrame);

  // Set viewport and scissor
  setupDefaultVieportAndScissor(commandBuffer, m_Renderer);

  // Bind and draw mesh
  m_Mesh->bind(commandBuffer);
  m_Mesh->draw(commandBuffer);

  // End render pass
  renderPass->end(commandBuffer);
}

void CubeSample::cleanup() {
  LOGFN;
  // Let RAII handle the resources

  m_Descriptor.reset();
  m_DescriptorPool.reset();
  m_DescriptorSetLayout.reset();

  // Clean up uniform buffers
  for (auto& ubo : m_UniformBuffers) {
    ubo.reset();
  }
  m_UniformBuffers.clear();

  // Clean up texture and mesh
  m_Texture.reset();
  m_Mesh.reset();
}

}  // namespace glint