#include "Mesh.h"

#include "Logger.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"

Mesh::Mesh(const std::string& filename) { loadMesh(filename); }

Mesh::~Mesh() {}

void Mesh::loadMesh(const std::string& filename) {
  //   tinyobj::attrib_t attrib;
  tinyobj::ObjReaderConfig reader_config;
  reader_config.mtl_search_path = "./";  // Path to material files

  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(filename, reader_config)) {
    if (!reader.Error().empty()) {
      Logger::error("TinyObjReader error: ", reader.Error());
    }
    throw std::runtime_error("Failed to load/parse .obj file");
    return;
  }

  if (!reader.Warning().empty()) {
    Logger::log("TinyObjReader warning: ", reader.Warning());
  }

  auto& attrib = reader.GetAttrib();
  auto& shapes = reader.GetShapes();
  auto& materials = reader.GetMaterials();

  for (const auto& shape : shapes) {
    for (const auto& index : shape.mesh.indices) {
      vertices.push_back(attrib.vertices[3 * index.vertex_index + 0]);
      vertices.push_back(attrib.vertices[3 * index.vertex_index + 1]);
      vertices.push_back(attrib.vertices[3 * index.vertex_index + 2]);

      if (!attrib.normals.empty()) {
        normals.push_back(attrib.normals[3 * index.normal_index + 0]);
        normals.push_back(attrib.normals[3 * index.normal_index + 1]);
        normals.push_back(attrib.normals[3 * index.normal_index + 2]);
      }

      if (!attrib.texcoords.empty()) {
        texCoords.push_back(attrib.texcoords[2 * index.texcoord_index + 0]);
        texCoords.push_back(attrib.texcoords[2 * index.texcoord_index + 1]);
      }

      indices.push_back(indices.size());
    }
  }
}