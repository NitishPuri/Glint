// Texture.h
#pragma once
#include <string>

class Texture {
 public:
  Texture(const std::string& path);
  ~Texture();

  void bind(unsigned int slot = 0) const;
  void unbind() const;

 private:
  unsigned int m_ID;
  int m_Width, m_Height, m_BPP;
};
