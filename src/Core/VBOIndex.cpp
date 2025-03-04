#include "VBOIndex.h"

#include <map>

void indexVBO_slow(const std::vector<glm::vec3>& in_vertices, const std::vector<glm::vec2>& in_uvs,
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

////

struct PackedVertex {
  glm::vec3 position;
  glm::vec2 uv;
  glm::vec3 normal;
  bool operator<(const PackedVertex that) const { return memcmp((void*)this, (void*)&that, sizeof(PackedVertex)) > 0; };
};

bool getSimilarVertexIndex_fast(PackedVertex& packed, std::map<PackedVertex, index_t>& VertexToOutIndex,
                                index_t& result) {
  std::map<PackedVertex, index_t>::iterator it = VertexToOutIndex.find(packed);
  if (it == VertexToOutIndex.end()) {
    return false;
  } else {
    result = it->second;
    return true;
  }
}

void indexVBO(const std::vector<glm::vec3>& in_vertices, const std::vector<glm::vec2>& in_uvs,
              const std::vector<glm::vec3>& in_normals,
              // outputs
              std::vector<index_t>& out_indices, std::vector<glm::vec3>& out_vertices, std::vector<glm::vec2>& out_uvs,
              std::vector<glm::vec3>& out_normals) {
  std::map<PackedVertex, index_t> VertexToOutIndex;

  // For each input vertex
  for (unsigned int i = 0; i < in_vertices.size(); i++) {
    PackedVertex packed = {in_vertices[i], in_uvs[i], in_normals[i]};

    // Try to find a similar vertex in out_XXXX
    index_t index;
    bool found = getSimilarVertexIndex_fast(packed, VertexToOutIndex, index);

    if (found) {  // A similar vertex is already in the VBO, use it instead !
      out_indices.push_back(index);
    } else {  // If not, it needs to be added in the output data.
      out_vertices.push_back(in_vertices[i]);
      out_uvs.push_back(in_uvs[i]);
      out_normals.push_back(in_normals[i]);
      index_t newindex = (index_t)out_vertices.size() - 1;
      out_indices.push_back(newindex);
      VertexToOutIndex[packed] = newindex;
    }
  }
}
