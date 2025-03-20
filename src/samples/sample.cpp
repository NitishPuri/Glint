#include "sample.h"

#include "logger.h"
#include "renderer/mesh_factory.h"
#include "renderer/renderer.h"

namespace glint {

Sample::Sample(const std::string& name) : m_Name(name) {}

////////////////////////////////////////
// Triangle Sample

TriangleSample::TriangleSample() : Sample("Triangle Sample") {}

void TriangleSample::init(Window* window, Renderer* renderer) {
  LOGFN;

  m_Renderer = renderer;
  m_Mesh = MeshFactory::createTriangle(renderer->getContext());
}

void TriangleSample::update(float deltaTime) {}

void TriangleSample::render(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
  m_Mesh->bind(commandBuffer);
  m_Mesh->draw(commandBuffer);
}

void TriangleSample::cleanup() {}

////////////////////////////////////////
// Quad Sample

QuadSample::QuadSample() : Sample("Quad Sample") {}

void QuadSample::init(Window* window, Renderer* renderer) {
  LOGFN;

  m_Renderer = renderer;
  m_Mesh = MeshFactory::createQuad(renderer->getContext());
}

void QuadSample::update(float deltaTime) {}

void QuadSample::render(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
  m_Mesh->bind(commandBuffer);
  m_Mesh->draw(commandBuffer);
}

void QuadSample::cleanup() {}

}  // namespace glint