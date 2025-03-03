#pragma once

#include "Renderer.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

struct CameraProps {
  float fov, aspect, near, far;
  glm::vec3 position, target, up;
};

inline CameraProps getDefaultCameraProps() {
  CameraProps props;
  props.fov = 45.0f;
  props.aspect = 4.0f / 3.0f;
  props.near = 0.1f;
  props.far = 100.0f;
  props.position = glm::vec3(4, 3, -3);
  props.target = glm::vec3(0, 0, 0);
  props.up = glm::vec3(0, 1, 0);
  return props;
}

inline glm::mat4 getViewProjectionMatrix(const CameraProps& props) {
  glm::mat4 Projection = glm::perspective(glm::radians(props.fov), props.aspect, props.near, props.far);
  glm::mat4 View = glm::lookAt(props.position, props.target, props.up);
  return Projection * View;
}

enum CameraMode {
  CAMERA_MODE_NONE,
  CAMERA_MODE_FREE,
  CAMERA_MODE_ORBIT,
};

class CameraController {
 public:
  CameraController(const CameraProps& props) : m_Props(props) { m_ViewProjection = getViewProjectionMatrix(m_Props); }

  void update(float deltaTime);

  void onMouseMove(float dx, float dy);
  //   void onMouseButton(int button, bool pressed);
  //   void onKey(int key, bool pressed);

  glm::mat4 getViewProjection() const { return m_ViewProjection; }

  void processInputs();

  CameraMode m_Mode = CAMERA_MODE_NONE;

 private:
  CameraProps m_Props;
  glm::mat4 m_ViewProjection;

  //   bool m_FirstMouse = true;
};