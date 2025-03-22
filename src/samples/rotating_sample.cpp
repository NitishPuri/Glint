// rotating_cube_sample.cpp
#include "rotating_sample.h"

#include <glm/gtc/matrix_transform.hpp>

#include "core/config.h"
#include "core/logger.h"
#include "core/window.h"
#include "renderer/mesh_factory.h"
#include "renderer/pipeline.h"
#include "renderer/render_pass.h"
#include "renderer/renderer.h"
#include "renderer/swapchain.h"

namespace glint {

RotatingSample::RotatingSample() : Sample("RotatingSample") { LOGFN; }

void RotatingSample::init(Window* window, Renderer* renderer) {
  LOGFN;
  m_Renderer = renderer;

  uint32_t framesInFlight = renderer->getFramesInFlight();
  LOG("Creating resources for", framesInFlight, "frames in flight");

  // Create a cube mesh
  //   m_Mesh = MeshFactory::createCube(renderer->getContext());
  m_Mesh = MeshFactory::createTriangle(renderer->getContext());

  // Create descriptor set layout
  m_DescriptorSetLayout = DescriptorSetLayout::Builder(renderer->getContext())
                              .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                              .build();

  PipelineConfig config;
  config.descriptorSetLayout = m_DescriptorSetLayout.get();
  config.vertexShaderPath = Config::getShaderFile("shader.vert");
  config.fragmentShaderPath = Config::getShaderFile("shader.frag");
  config.vertexFormat = VertexAttributeFlags::POSITION_COLOR;
  config.cullMode = VK_CULL_MODE_NONE;

  renderer->createPipeline(&config);

  // Create descriptor pool
  m_DescriptorPool =
      std::make_unique<DescriptorPool>(renderer->getContext(), m_DescriptorSetLayout.get(), framesInFlight);

  // Create uniform buffer
  m_UniformBuffers.resize(framesInFlight);
  for (auto& ubo : m_UniformBuffers) {
    ubo = std::make_unique<UniformBuffer>(renderer->getContext(), sizeof(UniformBufferObject));
  }

  // Create descriptor
  m_Descriptor = std::make_unique<Descriptor>(renderer->getContext(), m_DescriptorSetLayout.get(),
                                              m_DescriptorPool.get(), framesInFlight);

  // Update descriptor with uniform buffer
  //   m_Descriptor->updateUniformBuffer(0, m_UniformBuffer->getBuffer(), sizeof(UniformBufferObject));
  for (uint32_t i = 0; i < framesInFlight; i++) {
    m_Descriptor->updateUniformBuffer(m_UniformBuffers[i]->getBuffer(), sizeof(UniformBufferObject), 0, i);
  }

  // Set initial transformation matrices
  VkExtent2D extent = renderer->getSwapChain()->getExtent();
  float aspect = extent.width / (float)extent.height;

  // Set up initial MVP matrices
  m_UBOData.model = glm::mat4(1.0f);
  m_UBOData.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),   // Camera position
                               glm::vec3(0.0f, 0.0f, 0.0f),   // Look at position
                               glm::vec3(0.0f, 1.0f, 0.0f));  // Up vector
  m_UBOData.proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f);

  // Vulkan's Y coordinate is inverted compared to OpenGL
  m_UBOData.proj[1][1] *= -1;

  // Update the uniform buffer with initial data
  for (size_t i = 0; i < framesInFlight; i++) {
    m_UniformBuffers[i]->update(&m_UBOData, sizeof(m_UBOData));
  }
}

void RotatingSample::update(float deltaTime) {
  // Update rotation angle
  m_RotationAngle += deltaTime * 45.0f;  // 45 degrees per second
  if (m_RotationAngle > 360.0f) {
    m_RotationAngle -= 360.0f;
  }

  // Update model matrix with rotation
  m_UBOData.model = glm::rotate(glm::mat4(1.0f), glm::radians(m_RotationAngle), m_RotationAxis);

  uint32_t currentFrame = m_Renderer->getCurrentFrame();

  // Update uniform buffer with new matrices
  m_UniformBuffers[currentFrame]->update(&m_UBOData, sizeof(m_UBOData));
}

void RotatingSample::render(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
  LOGFN_ONCE;

  // Get components from renderer
  auto pipeline = m_Renderer->getPipeline();
  auto currentFrame = m_Renderer->getCurrentFrame();

  // Bind pipeline
  pipeline->bind(commandBuffer);

  // Bind descriptor set
  m_Descriptor->bind(commandBuffer, pipeline->getPipelineLayout(), currentFrame);

  // Bind mesh
  m_Mesh->bind(commandBuffer);

  setupDefaultVieportAndScissor(commandBuffer, m_Renderer);

  // Draw mesh
  m_Mesh->draw(commandBuffer);
}

void RotatingSample::cleanup() {
  LOGFN;

  //   if (m_Renderer) {
  //     m_Renderer->waitIdle();
  //   }

  m_Descriptor.reset();
  //   m_UniformBuffer.reset();
  for (auto& ubo : m_UniformBuffers) {
    ubo.reset();
  }
  m_DescriptorPool.reset();
  m_DescriptorSetLayout.reset();
  m_Mesh.reset();
}

}  // namespace glint