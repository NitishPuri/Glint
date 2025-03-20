#include "mesh_factory.h"

#include "../logger.h"
#include "vulkan_context.h"

namespace glint {

std::unique_ptr<Mesh> MeshFactory::createTriangle(VulkanContext* context) {
  LOGFN;

  const std::vector<Vertex> vertices = {{{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                                        {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                                        {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

  const std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

  return std::make_unique<Mesh>(context, vertices, indices);
}

std::unique_ptr<Mesh> MeshFactory::createQuad(VulkanContext* context) {
  LOGFN;

  std::vector<Vertex> vertices = {{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                                  {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                                  {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                                  {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}};

  std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

  return std::make_unique<Mesh>(context, vertices, indices);
}

std::unique_ptr<Mesh> MeshFactory::createCube(VulkanContext* context) {
  LOGFN;

  std::vector<Vertex> vertices = {
      // Front face
      {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},  // 0
      {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},   // 1
      {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},    // 2
      {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}},   // 3

      // Back face
      {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},  // 4
      {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}},   // 5
      {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},    // 6
      {{-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}},   // 7
  };

  std::vector<uint32_t> indices = {// Front face
                                   0, 1, 2, 2, 3, 0,

                                   // Right face
                                   1, 5, 6, 6, 2, 1,

                                   // Back face
                                   5, 4, 7, 7, 6, 5,

                                   // Left face
                                   4, 0, 3, 3, 7, 4,

                                   // Top face
                                   3, 2, 6, 6, 7, 3,

                                   // Bottom face
                                   4, 5, 1, 1, 0, 4};

  return std::make_unique<Mesh>(context, vertices, indices);
}

}  // namespace glint