#pragma once

#include <string>
#include <vector>

class Mesh {
 public:
  Mesh(const std::string& filename);
  ~Mesh();

  const std::vector<float>& getVertices() const { return vertices; }
  const std::vector<float>& getNormals() const { return normals; }
  const std::vector<float>& getTexCoords() const { return texCoords; }
  const std::vector<unsigned int>& getIndices() const { return indices; }

 private:
  void loadMesh(const std::string& filename);

  std::vector<float> vertices;
  std::vector<float> normals;
  std::vector<float> texCoords;
  std::vector<unsigned int> indices;
};