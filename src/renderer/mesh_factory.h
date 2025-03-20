#pragma once

#include <memory>

#include "mesh.h"

namespace glint {

class VulkanContext;

class MeshFactory {
 public:
  static std::unique_ptr<Mesh> createTriangle(VulkanContext* context);
  static std::unique_ptr<Mesh> createQuad(VulkanContext* context);
  static std::unique_ptr<Mesh> createCube(VulkanContext* context);
};

}  // namespace glint