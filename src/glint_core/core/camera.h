#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace glint {

class Camera {
 public:
  Camera(float aspectRatio = 1.0f, float fov = 45.0f, float nearPlane = 0.1f, float farPlane = 100.0f);

  void setAspectRatio(float aspectRatio);
  void setPosition(float x, float y, float z);
  void setTarget(float x, float y, float z);

  // Handle mouse movement for camera rotation/panning
  void processMouseMovement(int x0, int y0, int x1, int y1, bool rightButtonPressed, bool middleButtonPressed);

  // Handle mouse scroll for zooming
  void processMouseScroll(int scrollDelta) { m_ScrollDelta = scrollDelta; }

  // Update delta time for smooth movements
  void update(float deltaTime) { m_DeltaTime = deltaTime; }

  // Set screen dimensions for arcball calculations
  void setScreenDimensions(int width, int height);

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
  void updateViewMatrix();
  void updateProjectionMatrix();

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