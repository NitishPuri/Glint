#include "sample_manager.h"

#include <stdexcept>

#include "core/logger.h"
#include "renderer/render_pass.h"
#include "renderer/renderer.h"
#include "ui/imgui_manager.h"

// samples
#include "cube_sample.h"
#include "rotating_sample.h"
#include "sample.h"
#include "textured_quad.h"

namespace glint {

void SampleManager::init(Window* window, Renderer* renderer) {
  LOGFN;
  m_Window = window;
  m_Renderer = renderer;

  registerAllSamples();
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

void SampleManager::registerAllSamples() {
  registerSample(std::make_unique<glint::TriangleSample>());
  registerSample(std::make_unique<glint::QuadSample>());
  registerSample(std::make_unique<glint::RotatingSample>());
  registerSample(std::make_unique<glint::TexturedRotatingSample>());
  registerSample(std::make_unique<glint::CubeSample>());
}

void SampleManager::registerSample(std::unique_ptr<Sample> sample) {
  LOGFN;
  std::string name = sample->getName();
  LOG("Registering sample:", name);

  // Store the sample
  m_Samples[name] = std::move(sample);
  m_SampleNames.push_back(name);

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
  auto renderPass = m_Renderer->getRenderPass();
  // auto pipeline = m_Renderer->getPipeline();

  // Begin render pass (moved from Sample)

  ImGui::Begin("Sample Selector");
  if (ImGui::BeginCombo("Samples", m_ActiveSample->getName().c_str())) {
    for (auto& name : m_SampleNames) {
      bool isSelected = (m_ActiveSample->getName() == name);
      if (ImGui::Selectable(name.c_str(), isSelected)) {
        setActiveSample(name);
      }
      if (isSelected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  ImGui::End();

  renderPass->begin(commandBuffer, imageIndex, {0.1f, 0.1f, 0.2f, 1.0f});
  if (m_ActiveSample) {
    m_ActiveSample->render(commandBuffer, imageIndex);
  }

  ImGuiManager::render(commandBuffer);

  renderPass->end(commandBuffer);
}

}  // namespace glint