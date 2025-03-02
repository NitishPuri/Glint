// VertexBuffer.h
#pragma once

#include "Renderer.h"

class VertexBuffer {
 public:
  VertexBuffer(const void* data, unsigned int size) {
    glGenBuffers(1, &m_ID);
    glBindBuffer(GL_ARRAY_BUFFER, m_ID);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  }
  ~VertexBuffer() { glDeleteBuffers(1, &m_ID); }

  void bind() const { glBindBuffer(GL_ARRAY_BUFFER, m_ID); }
  void unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }

 private:
  unsigned int m_ID;
};
