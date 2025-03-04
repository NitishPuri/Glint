#include "VBOIndex.h"

void indexVBO(const std::vector<glm::vec3>& in_vertices, const std::vector<glm::vec2>& in_uvs,
              const std::vector<glm::vec3>& in_normals,
              // outputs
              std::vector<unsigned int>& out_indices, std::vector<glm::vec3>& out_vertices,
              std::vector<glm::vec2>& out_uvs, std::vector<glm::vec3>& out_normals) {
  out_indices.clear();
  out_vertices.clear();
  out_uvs.clear();
  out_normals.clear();
  // For each input vertex
  for (unsigned int i = 0; i < in_vertices.size(); i++) {
    // Try to find a similar vertex in out_XXXX
    unsigned short index;
    bool found = false;
    for (unsigned int j = 0; j < out_vertices.size(); j++) {
      if (in_vertices[i] == out_vertices[j] && in_uvs[i] == out_uvs[j] && in_normals[i] == out_normals[j]) {
        index = j;
        found = true;
        break;
      }
    }
    if (found) {
      out_indices.push_back(index);
    } else {
      out_vertices.push_back(in_vertices[i]);
      out_uvs.push_back(in_uvs[i]);
      out_normals.push_back(in_normals[i]);
      out_indices.push_back((unsigned short)out_vertices.size() - 1);
    }
  }
}