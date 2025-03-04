#pragma once

#include <glm/glm.hpp>
#include <vector>

using index_t = unsigned int;

void indexVBO(const std::vector<glm::vec3>& in_vertices, const std::vector<glm::vec2>& in_uvs,
              const std::vector<glm::vec3>& in_normals,
              // outputs
              std::vector<unsigned int>& out_indices, std::vector<glm::vec3>& out_vertices,
              std::vector<glm::vec2>& out_uvs, std::vector<glm::vec3>& out_normals);