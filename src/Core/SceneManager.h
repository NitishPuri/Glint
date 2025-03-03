#include <functional>
#include <memory>
#include <unordered_map>

#include "SceneBase.h"

class SceneManager {
 public:
  SceneManager() {}
  ~SceneManager() {}

  void registerScene(const std::string& name, std::function<std::shared_ptr<SceneBase>()> scene) {
    m_Scenes[name] = scene;
  }

  void loadScene(const std::string& name) {
    if (m_CurrentScene) {
      m_CurrentScene->onDetach();
    }
    auto it = m_Scenes.find(name);
    if (it != m_Scenes.end()) {
      // create scene
      m_CurrentScene = it->second();
      m_CurrentScene->onAttach();
    }
  }

  void onUpdate(float deltaTime) { 
    if (m_CurrentScene) {
      m_CurrentScene->onUpdate(deltaTime); 
    }
  }

  void onRender() { m_CurrentScene->onRender(); }

  void onImGuiRender() { m_CurrentScene->onImGuiRender(); }

 private:
  std::unordered_map<std::string, std::function<std::shared_ptr<SceneBase>()>> m_Scenes;
  std::shared_ptr<SceneBase> m_CurrentScene;
};