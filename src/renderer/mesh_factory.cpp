#include "mesh_factory.h"

#include "../logger.h"
#include "vk_context.h"

namespace glint {

std::unique_ptr<Mesh> MeshFactory::createTriangle(VkContext* context) {
  LOGFN;

  const std::vector<Vertex> vertices = {{{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                                        {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                                        {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}}};

  //   const std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};
  const std::vector<uint32_t> indices = {0, 1, 2};
  //   const std::vector<uint32_t> indices = {0, 2, 1};

  return std::make_unique<Mesh>(context, vertices, indices);
}

std::unique_ptr<Mesh> MeshFactory::createQuad(VkContext* context, bool textured) {
  LOGFN;

  std::vector<Vertex> vertices = {{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                                  {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                                  {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                                  {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}};
  if (textured) {
    vertices = {{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
                {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
                {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
                {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}};
  }

  std::vector<uint32_t> indices = {0, 2, 1, 3, 2, 0};

  return std::make_unique<Mesh>(context, vertices, indices);
}

std::unique_ptr<Mesh> MeshFactory::createCube(VkContext* context) {
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

std::unique_ptr<Mesh> MeshFactory::createTexturedCube(VkContext* context) {
  LOGFN;

  // For a proper textured cube, we need separate vertices for each face
  // even if they share the same position in space
  std::vector<Vertex> vertices = {
      // Front face (z = 0.5)
      {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},  // bottom-left
      {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},   // bottom-right
      {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},    // top-right
      {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},   // top-left

      // Back face (z = -0.5)
      {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},  // bottom-right
      {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},   // bottom-left
      {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},    // top-left
      {{-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},   // top-right

      // Right face (x = 0.5)
      {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},   // bottom-left
      {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},  // bottom-right
      {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},   // top-right
      {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},    // top-left

      // Left face (x = -0.5)
      {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},  // bottom-left
      {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},   // bottom-right
      {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},    // top-right
      {{-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},   // top-left

      // Top face (y = 0.5)
      {{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},   // bottom-left
      {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},    // bottom-right
      {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},   // top-right
      {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},  // top-left

      // Bottom face (y = -0.5)
      {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},  // bottom-left
      {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},   // bottom-right
      {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},    // top-right
      {{-0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}}    // top-left
  };

  // With 24 vertices (4 per face), we use simple quad indices for each face
  std::vector<uint32_t> indices = {// Front face
                                   0, 1, 2, 2, 3, 0,

                                   // Back face
                                   4, 5, 6, 6, 7, 4,

                                   // Right face
                                   8, 9, 10, 10, 11, 8,

                                   // Left face
                                   12, 13, 14, 14, 15, 12,

                                   // Top face
                                   16, 17, 18, 18, 19, 16,

                                   // Bottom face
                                   20, 21, 22, 22, 23, 20};

  return std::make_unique<Mesh>(context, vertices, indices, VertexAttributeFlags::POSITION_COLOR_TEXCOORD);
}

}  // namespace glint