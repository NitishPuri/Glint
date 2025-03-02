// VertexArray.h
#pragma once
#include "Renderer.h"

class VertexArray {
 public:
  VertexArray() { glGenVertexArrays(1, &m_ID); }
  ~VertexArray() { glDeleteVertexArrays(1, &m_ID); }

  void bind() const { glBindVertexArray(m_ID); }
  void unbind() const { glBindVertexArray(0); }

 private:
  unsigned int m_ID;
};
