#include "Camera.h"

void CameraController::update(float deltaTime) { m_ViewProjection = getViewProjectionMatrix(m_Props); }