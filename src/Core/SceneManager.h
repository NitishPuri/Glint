#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include "Renderer.h"
#include "SceneBase.h"

class SceneManager {
 public:
  SceneManager(int width, int height) : m_Width(width), m_Height(height) {}
  ~SceneManager() {}

  void registerScene(const std::string& name, std::function<std::shared_ptr<SceneBase>()> scene) {
    m_Scenes[name] = scene;
  }

  void loadScene(const std::string& name) {
    if (m_CurrentScene) {
      m_CurrentScene->onDetach();
    }
    if (name.empty()) {
      m_CurrentScene = nullptr;
      return;
    }
    auto it = m_Scenes.find(name);
    if (it != m_Scenes.end()) {
      // create scene
      m_CurrentScene = it->second();
      m_CurrentScene->onAttach(m_Width, m_Height);
    }
  }

  void onUpdate(float deltaTime) {
    // Logger::log("SceneManager::onUpdate : ", deltaTime);
    if (m_CurrentScene) {
      m_CurrentScene->onUpdate(deltaTime);
    }
  }

  void onRender() {
    if (m_CurrentScene)
      m_CurrentScene->onRender();
    else {
      // clear the screen
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);
    }
  }

  void onImGuiRender() {
    ImGui::Begin("Scene Control Panel");
    if (ImGui::BeginCombo("Scene", m_CurrentScene ? m_CurrentScene->getName().c_str() : "Select Scene")) {
      for (const auto& pair : m_Scenes) {
        bool is_selected = (m_CurrentScene && m_CurrentScene->getName() == pair.first);
        if (ImGui::Selectable(pair.first.c_str(), is_selected)) {
          loadScene(pair.first);
        }
        if (is_selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      if (ImGui::Selectable("<none>", m_CurrentScene == nullptr)) {
        loadScene("");
      }

      ImGui::EndCombo();
    }

    ImGui::End();

    if (m_CurrentScene) {
      m_CurrentScene->onImGuiRender();
    }
  }

  void onWindowResize(int width, int height) {
    m_Width = width;
    m_Height = height;
    if (m_CurrentScene) {
      m_CurrentScene->onWindowResize(width, height);
    }
  }

 private:
  int m_Width, m_Height;
  std::unordered_map<std::string, std::function<std::shared_ptr<SceneBase>()>> m_Scenes;
  std::shared_ptr<SceneBase> m_CurrentScene;
};