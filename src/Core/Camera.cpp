#include "Camera.h"

#include "Renderer.h"
// #include "imgui/im"

void CameraController::update(float deltaTime) {
  processInputs();
  m_ViewProjection = getViewProjectionMatrix(m_Props);
}

void CameraController::processInputs() {
  ImGuiIO& io = ImGui::GetIO();

  glm::vec3 forward = m_Props.target - m_Props.position;
  glm::vec3 right = glm::normalize(glm::cross(forward, m_Props.up));

  ImVec2 mouse_drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right, 0.01f);
  ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
  Logger::log("Mouse Drag: ", mouse_drag.x, ", ", mouse_drag.y);

  auto speed = 0.1f;
  auto dx = mouse_drag.x * speed;
  auto dy = mouse_drag.y * speed;

  if (m_Mode == CAMERA_MODE_FREE) {
    m_Props.position += forward * dy + right * dx;
    m_Props.target += forward * dy + right * dx;
  }

  m_ViewProjection = getViewProjectionMatrix(m_Props);

  //   onMouseMove(mouse_drag.x, mouse_drag.y);

  // Capture mouse buttons
  //   for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) {
  //     if (io.MouseDown[i]) {
  //       g_CameraController.onMouseButton(i, true);
  //     } else {
  //       g_CameraController.onMouseButton(i, false);
  //     }
  //   }

  //   // Capture keyboard keys
  //   for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) {
  //     if (io.KeysDown[i]) {
  //       g_CameraController.onKey(i, true);
  //     } else {
  //       g_CameraController.onKey(i, false);
  //     }
  //   }
}

void CameraController::onMouseMove(float dx, float dy) {
  //   float xOffset = x - m_LastMouseX;
  //   float yOffset = m_LastMouseY - y;  // Reversed since y-coordinates go from bottom to top

  //   m_LastMouseX = x;
  //   m_LastMouseY = y;

  //   float sensitivity = 0.1f;
  //   xOffset *= sensitivity;
  //   yOffset *= sensitivity;

  //   m_Yaw += xOffset;
  //   m_Pitch += yOffset;

  //   if (m_Pitch > 89.0f) m_Pitch = 89.0f;
  //   if (m_Pitch < -89.0f) m_Pitch = -89.0f;

  //   updateCameraVectors();
}