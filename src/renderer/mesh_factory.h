#pragma once

#include <memory>

#include "mesh.h"

namespace glint {

class VkContext;

class MeshFactory {
 public:
  static std::unique_ptr<Mesh> createTriangle(VkContext* context);
  static std::unique_ptr<Mesh> createQuad(VkContext* context, bool textured = false);
  static std::unique_ptr<Mesh> createCube(VkContext* context);
  static std::unique_ptr<Mesh> createTexturedCube(VkContext* context);
};

}  // namespace glint