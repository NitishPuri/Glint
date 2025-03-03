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

  static int cntr = 0;
  cntr++;

  //   if (cntr % 100 == 0) {
  //     Logger::log("Cam position: ", m_Props.position);
  //     Logger::log("Cam  target: ", m_Props.target);

  //     glm::vec3 direction = m_Props.position - m_Props.target;
  //     auto radius = glm::length(direction);
  //     auto theta = atan2(direction.z, direction.x);
  //     auto phi = acos(direction.y / radius);

  //     Logger::log("Cam  direction: ", direction);
  //     Logger::log("Cam  radius: ", radius);
  //     Logger::log("Cam  theta: ", theta, " phi : ", phi);
  //   }

  if (mouse_drag.x == 0 && mouse_drag.y == 0) {
    return;
  }

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
    auto radius = glm::length(direction);

    auto theta = atan2(direction.z, direction.x);
    auto phi = acos(direction.y / radius);

    // TODO: Better camera conmtrols needed
    //  if (cntr % 10 == 0) {
    //    Logger::log("Orbit mode position: ", m_Props.position);
    //    Logger::log("Orbit mode target: ", m_Props.target);
    //    Logger::log("Orbit mode direction: ", direction);
    //    Logger::log("Orbit mode radius: ", radius);
    //    Logger::log("Orbit mode theta: ", theta, " phi : ", phi);
    //  }

    theta += dx;
    phi = std::clamp(phi + dy, 0.1f, 3.13f);

    direction[0] = radius * sin(phi) * cos(theta);
    direction[1] = radius * cos(phi);
    direction[2] = radius * sin(phi) * sin(theta);

    m_Props.position = m_Props.target + direction;

    // if (cntr % 10 == 0) {
    //   Logger::log("Orbit mode dx: ", dx, " dy: ", dy);
    //   Logger::log("Orbit mode theta: ", theta, " phi : ", phi);
    //   Logger::log("Orbit mode direction: ", direction);
    //   Logger::log("Orbit mode position: ", m_Props.position);
    //   Logger::log("Orbit mode target: ", m_Props.target);
    //   Logger::log("DONE");
    // }
  }
}
