#pragma once

#include <string>
#include <vector>

#include "glm/glm.hpp"

class Mesh {
 public:
  Mesh(const std::string& filename);
  ~Mesh();

  const std::vector<glm::vec3>& getVertices() const { return vertices; }
  const std::vector<glm::vec3>& getNormals() const { return normals; }
  const std::vector<glm::vec2>& getTexCoords() const { return texCoords; }
  const std::vector<unsigned int>& getIndices() const { return indices; }

  const int getTriangleCount() const { return int(indices.size() / 3); }

 private:
  void loadMesh(const std::string& filename);

  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> texCoords;
  std::vector<unsigned int> indices;
};