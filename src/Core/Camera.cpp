#include "Camera.h"

#include <algorithm>

#include "Renderer.h"

void CameraController::update(float deltaTime) {
  processInputs();
  m_ViewProjection = getViewProjectionMatrix(m_Props);
}

void CameraController::processInputs() {
  ImGuiIO& io = ImGui::GetIO();

  ImVec2 mouse_drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right, 0.01f);
  ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);

  auto speed = 0.1f;
  auto dx = mouse_drag.x * speed;
  auto dy = mouse_drag.y * speed;

  if (m_Mode == CAMERA_MODE_FREE) {
    glm::vec3 forward = m_Props.target - m_Props.position;
    glm::vec3 right = glm::normalize(glm::cross(forward, m_Props.up));

    m_Props.position += forward * dy + right * dx;
    m_Props.target += forward * dy + right * dx;

  } else if (m_Mode == CAMERA_MODE_ORBIT) {
    glm::vec3 direction = m_Props.position - m_Props.target;
    auto radius = direction.length();

    auto theta = atan2(direction.z, direction.x);
    auto phi = acos(direction.y / radius);

    theta += dx;
    phi = std::clamp(phi + dy, 0.1f, 3.13f);

    direction[0] = radius * sin(phi) * cos(theta);
    direction[1] = radius * cos(phi);
    direction[2] = radius * sin(phi) * sin(theta);

    m_Props.position = m_Props.target + direction;
  }

  m_ViewProjection = getViewProjectionMatrix(m_Props);
}
