
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
    return;
  }

  void genDepthRenderBuffer() {
    // GLuint depthrenderbuffer;
    glGenRenderbuffers(1, &m_depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBuffer);
  }

  void bind() { glBindFramebuffer(GL_FRAMEBUFFER, m_fbo); }
  void unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

  //   void bindTexture(unsigned int slot = 0) const;
  //   void unbindTexture() const;
  //   unsigned int getTextureID() const { return m_TextureID; }

  const unique_ptr<Texture>& getColorAttachment() { return m_ColorAttachment; }
  // Texture* getDepthAttachment() { return m_DepthAttachment.get(); }

 private:
  GLuint m_fbo;

  unique_ptr<Texture> m_ColorAttachment;
  // unique_ptr<Texture> m_DepthAttachment;

  int m_Width = -1, m_Height = -1;
  GLuint m_depthRenderBuffer;
};