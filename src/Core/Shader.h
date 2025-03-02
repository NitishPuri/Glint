// Shader.h
#pragma once
#include <string>
#include <unordered_map>

class Shader {
 public:
  Shader() {}
  Shader(const std::string& vertexPath, const std::string& fragmentPath);
  ~Shader();

  void init(const std::string& vertexPath, const std::string& fragmentPath);

  void bind() const;
  void unbind() const;

  void setUniform1i(const std::string& name, int value);
  void setUniform1f(const std::string& name, float value);
  void setUniform3f(const std::string& name, float x, float y, float z);
  void setUniform4f(const std::string& name, float x, float y, float z,
                    float w);
  //   void setUniformMat4(const std::string& name, const glm::mat4& matrix);

 private:
  unsigned int m_ID;
  std::unordered_map<std::string, int> m_UniformLocationCache;
  int getUniformLocation(const std::string& name);
};

// GLuint LoadShaders(const char* vertex_file_path,
//                    const char* fragment_file_path);