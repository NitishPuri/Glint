#include "camera.h"

// Include the arcball camera implementation once
#define ARCBALL_CAMERA_IMPLEMENTATION
#include "arcball_camera.h"

namespace glint {

Camera::Camera(float aspectRatio, float fov, float nearPlane, float farPlane)
    : m_AspectRatio(aspectRatio), m_Fov(fov), m_NearPlane(nearPlane), m_FarPlane(farPlane) {
  // Initialize default camera position
  m_Eye[0] = 0.0f;
  m_Eye[1] = 0.0f;
  m_Eye[2] = 3.0f;

  // Initialize target position (origin)
  m_Target[0] = 0.0f;
  m_Target[1] = 0.0f;
  m_Target[2] = 0.0f;

  // Initialize up vector
  m_Up[0] = 0.0f;
  m_Up[1] = 1.0f;
  m_Up[2] = 0.0f;

  // Initialize view matrix
  updateViewMatrix();
  updateProjectionMatrix();
}

void Camera::setAspectRatio(float aspectRatio) {
  m_AspectRatio = aspectRatio;
  updateProjectionMatrix();
}

void Camera::setPosition(float x, float y, float z) {
  m_Eye[0] = x;
  m_Eye[1] = y;
  m_Eye[2] = z;
  updateViewMatrix();
}

void Camera::setTarget(float x, float y, float z) {
  m_Target[0] = x;
  m_Target[1] = y;
  m_Target[2] = z;
  updateViewMatrix();
}

void Camera::processMouseMovement(int x0, int y0, int x1, int y1, bool rightButtonPressed, bool middleButtonPressed) {
  m_LastMouseX = m_MouseX;
  m_LastMouseY = m_MouseY;
  m_MouseX = x1;
  m_MouseY = y1;
  m_RightButtonPressed = rightButtonPressed;
  m_MiddleButtonPressed = middleButtonPressed;

  // Call C arcball camera update
  arcball_camera_update(m_Eye, m_Target, m_Up, m_ViewMatrix,
                        m_DeltaTime,                                     // Delta time in seconds
                        m_ZoomPerScroll,                                 // How much to zoom per scroll tick
                        m_PanSpeed,                                      // Pan speed
                        m_RotationMultiplier,                            // Rotation speed multiplier
                        m_ScreenWidth, m_ScreenHeight,                   // Screen dimensions
                        m_LastMouseX, m_MouseX, m_LastMouseY, m_MouseY,  // Mouse coordinates
                        m_MiddleButtonPressed ? 1 : 0,                   // Middle button for panning
                        m_RightButtonPressed ? 1 : 0,                    // Right button for rotation
                        m_ScrollDelta,                                   // Scroll wheel delta
                        0                                                // Flags
  );

  m_ScrollDelta = 0;  // Reset scroll delta after processing

  // Update projection matrix (view matrix is updated by arcball_camera_update)
  updateProjectionMatrix();
}

void Camera::setScreenDimensions(int width, int height) {
  m_ScreenWidth = width;
  m_ScreenHeight = height;

  // Update aspect ratio
  m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);
  updateProjectionMatrix();
}

void Camera::updateViewMatrix() {
  // Get the look direction
  float look[3] = {m_Target[0] - m_Eye[0], m_Target[1] - m_Eye[1], m_Target[2] - m_Eye[2]};

  // Normalize look vector
  float length = sqrtf(look[0] * look[0] + look[1] * look[1] + look[2] * look[2]);
  if (length > 0.0001f) {
    look[0] /= length;
    look[1] /= length;
    look[2] /= length;

    // Calculate view matrix
    arcball_camera_look_to(m_Eye, look, m_Up, m_ViewMatrix, 0);
  }
}

void Camera::updateProjectionMatrix() {
  m_ProjectionMatrix = glm::perspective(glm::radians(m_Fov), m_AspectRatio, m_NearPlane, m_FarPlane);
  // Flip Y for Vulkan coordinate system
  m_ProjectionMatrix[1][1] *= -1;
}

}  // namespace glint