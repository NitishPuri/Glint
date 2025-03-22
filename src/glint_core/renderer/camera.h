#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Include the arcball camera implementation once
#define ARCBALL_CAMERA_IMPLEMENTATION
#include "arcball_camera.h"

namespace glint {

class ArcballCamera {
 public:
  ArcballCamera(float aspectRatio = 1.0f, float fov = 45.0f, float nearPlane = 0.1f, float farPlane = 100.0f)
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

  void setAspectRatio(float aspectRatio) {
    m_AspectRatio = aspectRatio;
    updateProjectionMatrix();
  }

  void setPosition(float x, float y, float z) {
    m_Eye[0] = x;
    m_Eye[1] = y;
    m_Eye[2] = z;
    updateViewMatrix();
  }

  void setTarget(float x, float y, float z) {
    m_Target[0] = x;
    m_Target[1] = y;
    m_Target[2] = z;
    updateViewMatrix();
  }

  // Handle mouse movement for camera rotation/panning
  void processMouseMovement(int x0, int y0, int x1, int y1, bool rightButtonPressed, bool middleButtonPressed) {
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

    // m_ScrollDelta = 0;  // Reset scroll delta after processing

    // Update projection matrix (view matrix is updated by arcball_camera_update)
    updateProjectionMatrix();
  }

  // Handle mouse scroll for zooming
  void processMouseScroll(int scrollDelta) { m_ScrollDelta = scrollDelta; }

  // Update delta time for smooth movements
  void update(float deltaTime) { m_DeltaTime = deltaTime; }

  // Set screen dimensions for arcball calculations
  void setScreenDimensions(int width, int height) {
    m_ScreenWidth = width;
    m_ScreenHeight = height;

    // Update aspect ratio
    m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);
    updateProjectionMatrix();
  }

  // Getters for the matrices
  glm::mat4 getViewMatrix() const { return glm::make_mat4(m_ViewMatrix); }

  glm::mat4 getProjectionMatrix() const { return m_ProjectionMatrix; }

  // Get camera parameters
  glm::vec3 getPosition() const { return glm::vec3(m_Eye[0], m_Eye[1], m_Eye[2]); }

  glm::vec3 getTarget() const { return glm::vec3(m_Target[0], m_Target[1], m_Target[2]); }

  glm::vec3 getUp() const { return glm::vec3(m_Up[0], m_Up[1], m_Up[2]); }

  // Settings
  void setZoomSpeed(float speed) { m_ZoomPerScroll = speed; }
  void setPanSpeed(float speed) { m_PanSpeed = speed; }
  void setRotationSpeed(float speed) { m_RotationMultiplier = speed; }

 private:
  void updateViewMatrix() {
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

  void updateProjectionMatrix() {
    m_ProjectionMatrix = glm::perspective(glm::radians(m_Fov), m_AspectRatio, m_NearPlane, m_FarPlane);
    // Flip Y for Vulkan coordinate system
    m_ProjectionMatrix[1][1] *= -1;
  }

 private:
  // Camera parameters
  float m_Eye[3] = {0.0f, 0.0f, 3.0f};
  float m_Target[3] = {0.0f, 0.0f, 0.0f};
  float m_Up[3] = {0.0f, 1.0f, 0.0f};
  float m_ViewMatrix[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

  // Projection parameters
  glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
  float m_AspectRatio = 1.0f;
  float m_Fov = 45.0f;
  float m_NearPlane = 0.1f;
  float m_FarPlane = 100.0f;

  // Mouse and control parameters
  int m_MouseX = 0;
  int m_MouseY = 0;
  int m_LastMouseX = 0;
  int m_LastMouseY = 0;
  int m_ScrollDelta = 0;
  bool m_RightButtonPressed = false;
  bool m_MiddleButtonPressed = false;
  int m_ScreenWidth = 800;
  int m_ScreenHeight = 600;

  // Movement parameters
  float m_DeltaTime = 0.016f;  // Default to 60fps
  float m_ZoomPerScroll = 0.1f;
  float m_PanSpeed = 2.0f;
  float m_RotationMultiplier = 1.0f;
};

}  // namespace glint