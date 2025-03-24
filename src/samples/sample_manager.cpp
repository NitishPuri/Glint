#include "sample_manager.h"

#include <stdexcept>

#include "core/logger.h"
#include "renderer/render_pass.h"
#include "renderer/renderer.h"
#include "ui/imgui_manager.h"

// samples
#include "cube_sample.h"
#include "dynamic_uniform_buffer.h"
#include "rotating_sample.h"
#include "sample.h"
#include "textured_quad.h"

namespace glint {

SampleManager::SampleManager() {
  LOGFN;
  m_ActiveSample = nullptr;
  m_Window = nullptr;
  m_Renderer = nullptr;
  m_SampleNames.push_back("None");
  m_SampleCreators["None"] = []() { return nullptr; };
}

void SampleManager::init(Window* window, Renderer* renderer) {
  LOGFN;
  getInstance().m_Window = window;
  getInstance().m_Renderer = renderer;
}

void SampleManager::cleanup() {
  LOGFN;
  // Wait for GPU to finish operations
  auto& instance = getInstance();
  if (instance.m_Renderer) {
    instance.m_Renderer->waitIdle();
  }

  if (instance.m_ActiveSample) {
    LOG("Cleaning up active sample:", instance.m_ActiveSample->getName());
    instance.m_ActiveSample->cleanup();
    instance.m_ActiveSample.reset();
  }
}

void SampleManager::registerSample(const std::string& name, std::function<std::unique_ptr<Sample>()> createFn) {
  LOG("Registering sample:", name);

  // Store the sample
  m_SampleCreators[name] = std::move(createFn);
  m_SampleNames.push_back(name);

  // If this is the first sample, make it active
  if (!m_ActiveSample && m_Renderer) {
    setActiveSample(name);
  }
}

void SampleManager::setActiveSample(const std::string& name) {
  if (m_ActiveSample && m_ActiveSample->getName() == name) {
    LOG("Sample already active:", name);
    return;
  }

  // find sample top create
  auto sampleItr = m_SampleCreators.find(name);
  if (sampleItr == m_SampleCreators.end()) {
    LOG("Sample not found:", name);
    return;
  }

  if (m_Renderer) {
    m_Renderer->waitIdle();
  }

  // Clean up previous sample if any
  if (m_ActiveSample) {
    m_ActiveSample->cleanup();
    m_ActiveSample = nullptr;
  }

  // create new sample
  m_ActiveSample = sampleItr->second();
  LOG("Active sample set to:", name);

  // Initialize the new sample
  if (m_Window && m_Renderer && m_ActiveSample) {
    m_ActiveSample->init(m_Window, m_Renderer);
  }
}

void SampleManager::update(float deltaTime) {
  auto& instance = getInstance();
  if (instance.m_ActiveSample) {
    instance.m_ActiveSample->update(deltaTime);
  }
}

void SampleManager::renderSample(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
  auto renderPass = m_Renderer->getRenderPass();

  ImGui::Begin("Sample Selector");
  auto activeSampeName = m_ActiveSample ? m_ActiveSample->getName() : "None";
  if (ImGui::BeginCombo("Samples", activeSampeName.c_str())) {
    for (auto& name : m_SampleNames) {
      bool isSelected = (activeSampeName == name);
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