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
  m_Mesh = MeshFactory::createTriangle(renderer->getContext());
}

void BasicSample::update(float deltaTime) {}

void BasicSample::cleanup() {}

void BasicSample::render(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
  // TODO: Refactor this into a method in base sample
  auto renderPass = m_Renderer->getRenderPass();
  auto pipeline = m_Renderer->getPipeline();

  VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
  renderPass->begin(commandBuffer, imageIndex, clearColor);
  pipeline->bind(commandBuffer);

  setupDefaultVieportAndScissor(commandBuffer, m_Renderer);

  //   vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
  //                           &descriptorSets[currentFrame], 0, nullptr);

  m_Mesh->bind(commandBuffer);
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