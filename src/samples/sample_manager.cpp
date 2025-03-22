#include "sample_manager.h"

#include <stdexcept>

#include "core/logger.h"
#include "renderer/renderer.h"
#include "sample.h"

namespace glint {

void SampleManager::init(Window* window, Renderer* renderer) {
  LOGFN;
  m_Window = window;
  m_Renderer = renderer;
}

void SampleManager::cleanup() {
  LOGFN;
  // Wait for GPU to finish operations
  if (m_Renderer) {
    m_Renderer->waitIdle();
  }

  if (m_ActiveSample) {
    m_ActiveSample->cleanup();
  }

  // Clear all samples
  m_Samples.clear();
}

void SampleManager::registerSample(std::unique_ptr<Sample> sample) {
  LOGFN;
  std::string name = sample->getName();
  LOG("Registering sample:", name);

  // Store the sample
  m_Samples[name] = std::move(sample);

  // If this is the first sample, make it active
  if (!m_ActiveSample && m_Renderer) {
    setActiveSample(name);
  }
}

void SampleManager::setActiveSample(const std::string& name) {
  // Check if the sample exists
  auto it = m_Samples.find(name);
  if (it == m_Samples.end()) {
    LOG("Sample not found:", name);
    return;
  }

  if (m_ActiveSample && m_ActiveSample->getName() == name) {
    // LOG("Sample already active:", name);
    return;
  }

  if (m_Renderer) {
    m_Renderer->waitIdle();
  }

  // Clean up previous sample if any
  if (m_ActiveSample) {
    m_ActiveSample->cleanup();
  }

  // Set the new active sample
  m_ActiveSample = it->second.get();
  LOG("Active sample set to:", name);

  // Initialize the new sample
  if (m_Window && m_Renderer) {
    m_ActiveSample->init(m_Window, m_Renderer);
  }
}

void SampleManager::update(float deltaTime) {
  if (m_ActiveSample) {
    m_ActiveSample->update(deltaTime);
  }
}

void SampleManager::render(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
  if (m_ActiveSample) {
    m_ActiveSample->render(commandBuffer, imageIndex);
  }
}

}  // namespace glint