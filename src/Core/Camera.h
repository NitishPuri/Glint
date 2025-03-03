#pragma once

#include "Renderer.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

inline glm::mat4 getViewProjectionMatrix(float fov, float aspect, float near, float far, glm::vec3 position,
                                         glm::vec3 target, glm::vec3 up) {
  glm::mat4 Projection = glm::perspective(glm::radians(fov), aspect, near, far);

  // Camera matrix
  glm::mat4 View = glm::lookAt(position,           // Camera is at (4,3,-3), in World Space
                               target,             // and looks at the origin
                               glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
  );

  return Projection * View;
}
