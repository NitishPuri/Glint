
#pragma once

#include "Core/Renderer.h"
#include "Texture.h"

class FrameBuffer {
 public:
  ~FrameBuffer() {
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteRenderbuffers(1, &m_depthRenderBuffer);
  }

  void create(int width, int height) {
    m_Width = width;
    m_Height = height;
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  }

  void createColorAttachment() {
    m_ColorAttachment = make_unique<Texture>(m_Width, m_Height);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_ColorAttachment->getID(), 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return;
  }

  void genDepthRenderBuffer() {
    // GLuint depthrenderbuffer;
    glGenRenderbuffers(1, &m_depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBuffer);
  }

  void createDepthRenderTexture() {
    // cleanup ??
    // glDeleteRenderbuffers(1, &m_depthRenderBuffer); ??

    // GLuint depthTexture;
    glGenTextures(1, &m_depthRenderBuffer);
    glBindTexture(GL_TEXTURE_2D, m_depthRenderBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_Width, m_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthRenderBuffer, 0);
  }

  void bind() { glBindFramebuffer(GL_FRAMEBUFFER, m_fbo); }
  void unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

  const unique_ptr<Texture>& getColorAttachment() { return m_ColorAttachment; }
  // Texture* getDepthAttachment() { return m_DepthAttachment.get(); }

  GLuint getID() const { return m_fbo; }
  GLuint getDepthRenderBuffer() const { return m_depthRenderBuffer; }

 private:
  GLuint m_fbo;

  unique_ptr<Texture> m_ColorAttachment;
  // unique_ptr<Texture> m_DepthAttachment;

  int m_Width = -1, m_Height = -1;
  GLuint m_depthRenderBuffer;
};