// VertexBuffer.h
#pragma once

#include "Renderer.h"

class VertexBuffer {
 public:
  VertexBuffer(const void* data, unsigned int size) {
    GLCall(glGenBuffers(1, &m_ID));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_ID));
    GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
  }
  ~VertexBuffer() { glDeleteBuffers(1, &m_ID); }

  void bind() const { GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_ID)); }
  void unbind() const { GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0)); }

 private:
  unsigned int m_ID;
};
