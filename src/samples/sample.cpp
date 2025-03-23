#include "sample.h"

#include "core/config.h"
#include "core/logger.h"
#include "core/window.h"
#include "imgui.h"
#include "renderer/mesh_factory.h"
#include "renderer/pipeline.h"
#include "renderer/render_pass.h"
#include "renderer/renderer.h"
#include "renderer/swapchain.h"

namespace glint {

Sample::Sample(const std::string& name) : m_Name(name) {}

void Sample::init(Window* window, Renderer* renderer) {
  this->m_Window = window;
  this->m_Renderer = renderer;
  initSample(window, renderer);
}

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

void Sample::initCamera(float aspectRatio, float fov, float nearPlane, float farPlane) {
  m_Camera = std::make_unique<Camera>(aspectRatio, fov, nearPlane, farPlane);
  m_Camera->setPosition(0.0f, 0.0f, 2.0f);
  m_Camera->setTarget(0.0f, 0.0f, 0.0f);
}
Camera* Sample::getCamera() { return m_Camera.get(); }
void Sample::updateCamera(float deltaTime) { m_Camera->update(deltaTime); }
void Sample::processCameraInput() {
  ImGuiIO& io = ImGui::GetIO();

  // Process mouse movement
  VkExtent2D extent = m_Renderer->getSwapChain()->getExtent();
  // m_Camera->setScreenDimensions(m_Window->getWidth(), m_Window->getHeight());
  m_Camera->setScreenDimensions(extent.width, extent.height);
  m_Camera->processMouseScroll(static_cast<int>(io.MouseWheel));
  m_Camera->processMouseMovement(static_cast<int>(io.MousePosPrev.x), static_cast<int>(io.MousePosPrev.y),
                                 static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y), io.MouseDown[1],
                                 io.MouseDown[2]);
}

////////////////////////////////////////
// Basic Sample

BasicSample::BasicSample(const std::string& name) : Sample(name) {}

void BasicSample::initSample(Window* window, Renderer* renderer) {
  m_Renderer = renderer;

  PipelineConfig config;
  config.vertexShaderPath = Config::getShaderFile("basic.vert");
  config.fragmentShaderPath = Config::getShaderFile("shader.frag");
  config.descriptorSetLayout = nullptr;
  config.vertexFormat = VertexAttributeFlags::POSITION_COLOR;

  m_Renderer->createPipeline(&config);

  m_Mesh = MeshFactory::createTriangle(renderer->getContext());
}

void BasicSample::update(float deltaTime) {}

void BasicSample::cleanup() {}

void BasicSample::render(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
  auto pipeline = m_Renderer->getPipeline();
  pipeline->bind(commandBuffer);

  m_Mesh->bind(commandBuffer);
  setupDefaultVieportAndScissor(commandBuffer, m_Renderer);
  m_Mesh->draw(commandBuffer);
}

////////////////////////////////////////
// Triangle Sample

// TriangleSample::TriangleSample() : BasicSample("Triangle Sample") {}

////////////////////////////////////////
// Quad Sample

QuadSample::QuadSample() : BasicSample("Quad Sample") {}

void QuadSample::initSample(Window* window, Renderer* renderer) {
  BasicSample::initSample(window, renderer);
  setMesh(MeshFactory::createQuad(renderer->getContext()));
}

}  // namespace glint