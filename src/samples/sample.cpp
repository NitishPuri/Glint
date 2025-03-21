#include "sample.h"

#include "logger.h"
#include "renderer/mesh_factory.h"
#include "renderer/pipeline.h"
#include "renderer/render_pass.h"
#include "renderer/renderer.h"
#include "renderer/swapchain.h"

namespace glint {

Sample::Sample(const std::string& name) : m_Name(name) {}

void Sample::setupDefaultVieportAndScissor(VkCommandBuffer commandBuffer, Renderer* renderer) {
  auto swapChainExtent = renderer->getSwapChain()->getExtent();

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)swapChainExtent.width;
  viewport.height = (float)swapChainExtent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = swapChainExtent;
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

////////////////////////////////////////
// Basic Sample

BasicSample::BasicSample(const std::string& name) : Sample(name) {}

void BasicSample::init(Window* window, Renderer* renderer) {
  LOGFN;

  m_Renderer = renderer;

  PipelineConfig config;
  config.vertexShaderPath = getShaderPath("basic.vert");
  config.fragmentShaderPath = getShaderPath("shader.frag");
  config.descriptorSetLayout = nullptr;
  config.vertexFormat = VertexAttributeFlags::POSITION_COLOR;

  m_Renderer->createPipeline(&config);

  m_Mesh = MeshFactory::createTriangle(renderer->getContext());
}

void BasicSample::update(float deltaTime) {}

void BasicSample::cleanup() {}

void BasicSample::render(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
  // TODO: Refactor this into a method in base sample
  auto renderPass = m_Renderer->getRenderPass();
  auto pipeline = m_Renderer->getPipeline();

  renderPass->begin(commandBuffer, imageIndex, {0.0f, 0.0f, 0.0f, 1.0f});
  pipeline->bind(commandBuffer);

  m_Mesh->bind(commandBuffer);
  setupDefaultVieportAndScissor(commandBuffer, m_Renderer);
  m_Mesh->draw(commandBuffer);

  renderPass->end(commandBuffer);
}

////////////////////////////////////////
// Triangle Sample

// TriangleSample::TriangleSample() : BasicSample("Triangle Sample") {}

////////////////////////////////////////
// Quad Sample

QuadSample::QuadSample() : BasicSample("Quad Sample") {}

void QuadSample::init(Window* window, Renderer* renderer) {
  LOGFN;

  BasicSample::init(window, renderer);
  setMesh(MeshFactory::createQuad(renderer->getContext()));
}

}  // namespace glint