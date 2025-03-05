#pragma once

// core
#include "Core/SceneBase.h"
#include "Graphics/Camera.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/Mesh.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Graphics/VertexArray.h"
#include "Graphics/VertexBuffer.h"

// glm
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class RenderToTexture : public SceneBase {
 public:
  RenderToTexture() : SceneBase("Render To Texture"), m_CameraController(getDefaultCameraProps()) {
    m_CameraController.getProps().position = glm::vec3(0, 0, 5);
  }
  void onAttach() override {
    Logger::log("RenderToTexture::onAttach()\n\n");
    m_standarShader.init(getFilePath("/shaders/standard_shading.vert"), getFilePath("/shaders/standard_shading.frag"));

    // TODO: Move to something like Renderer::setup3D() ?
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    m_Mesh = std::make_unique<Mesh>(getFilePath("/res/suzanne.obj"));
    m_Mesh->index();

    // TODO: order matters here, does it?
    m_VertexArray = std::make_unique<VertexArray>();

    const auto &vertices = m_Mesh->getVertices();
    const auto &indices = m_Mesh->getIndices();
    const auto &tex_coords = m_Mesh->getTexCoords();
    const auto &normals = m_Mesh->getNormals();

    m_VertexBuffer = std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(glm::vec3));
    m_NormalBuffer = std::make_unique<VertexBuffer>(normals.data(), normals.size() * sizeof(glm::vec3));
    m_UVBuffer = std::make_unique<VertexBuffer>(tex_coords.data(), tex_coords.size() * sizeof(glm::vec2));
    m_IndexBuffer = std::make_unique<IndexBuffer>(indices.data(), int(indices.size()));

    m_Texture = std::make_unique<Texture>(getFilePath("/res/suzanne.jpg"));

    /// Setup RTT
    setupRtt();
  }

  void setupRtt() {
    glGenFramebuffers(1, &m_offscreenBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_offscreenBuffer);

    auto windowWidth = int(ImGui::GetIO().DisplaySize.x);
    auto windowHeight = int(ImGui::GetIO().DisplaySize.y);
    Logger::log("Window Width: ", windowWidth, " Window Height: ", windowHeight);
    if (windowWidth < 0 || windowHeight < 0) {
      // TODO: handle better, another way to ensure we get the right window size here?
      //  or maybe delay initialization until we have the window size?
      Logger::error("Window width or height is not set yet.");
      windowWidth = 800;
      windowHeight = 600;
    }
    // generate and bind texture we are going to render to
    m_RenderedTexture = std::make_unique<Texture>(windowWidth, windowHeight);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    // Poor filtering
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // The depth buffer
    GLuint depthrenderbuffer;
    glGenRenderbuffers(1, &depthrenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

    //// Alternative : Depth texture. Slower, but you can sample it later in your shader
    // GLuint depthTexture;
    // glGenTextures(1, &depthTexture);
    // glBindTexture(GL_TEXTURE_2D, depthTexture);
    // glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT24, 1024, 768, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Set "m_RenderedTexture" as our colour attachement #0
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_RenderedTexture->getID(), 0);

    //// Depth texture alternative :
    // glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers);  // "1" is the size of DrawBuffers

    // Always check that our framebuffer is ok
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      Logger::error("Framebuffer not complete!");
      return;
      // return false
    }

    // The fullscreen quad's FBO
    static const GLfloat g_quad_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f,  0.0f, 1.0f, -1.0f, 0.0f, 1.0f,  1.0f, 0.0f,
    };

    m_quadVertexBuffer = make_unique<VertexBuffer>(g_quad_vertex_buffer_data, sizeof(g_quad_vertex_buffer_data));
    // GLuint quad_vertexbuffer;
    // glGenBuffers(1, &quad_vertexbuffer);
    // glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

    // Create and compile our GLSL program from the shaders
    m_quadShader.init(getFilePath("/shaders/passthrough.vert"), getFilePath("/shaders/wobbly_texture.frag"));
    // m_RTTShader.bind();
    // m_RTTShader.setUniform1i("renderedTexture", 0);
    // GLuint texID = glGetUniformLocation(quad_programID, "renderedTexture");
    // GLuint timeID = glGetUniformLocation(quad_programID, "time");
    m_RTTInitialized = true;
  }

  void onDetach() override { glDisable(GL_CULL_FACE); };

  void onUpdate(float deltaTime) override {
    // Rotate Quad
    // m_Rotation += m_RotationSpeed * deltaTime * 10;
    // if (m_Rotation > 360.0f) m_Rotation -= 360.0f;

    // Update camera
    m_CameraController.update(deltaTime);
  };

  void onRender() override {
    // Render to our framebuffer
    {
      glBindFramebuffer(GL_FRAMEBUFFER, m_offscreenBuffer);
      glViewport(0, 0, int(ImGui::GetIO().DisplaySize.x), int(ImGui::GetIO().DisplaySize.y));

      // Clear the screen
      GLCall(glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.0f));
      GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

      // Use our shader
      m_standarShader.bind();

      glm::mat4 View = m_CameraController.getViewProjection();
      glm::mat4 Model = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0.5f, 1.0f, 0.0f));
      glm::mat4 MVP = View * Model;

      // Uniforms
      m_standarShader.bind();
      {
        // Camera
        m_standarShader.setUniformMat4("MVP", MVP);
        m_standarShader.setUniformMat4("V", View);
        m_standarShader.setUniformMat4("M", Model);

        // Lights
        m_standarShader.setUniform3f("LightPosition_worldspace", m_LightPos.x, m_LightPos.y, m_LightPos.z);
        m_standarShader.setUniform3f("LightColor", m_LightColor.x, m_LightColor.y, m_LightColor.z);
        m_standarShader.setUniform1f("LightPower", m_LightPower);

        // Material
        m_standarShader.setUniform1f("MaterialAmbient", m_AmbientStrength);
        m_standarShader.setUniform1f("MaterialSpecular", m_SpecularStrength);
      }

      m_standarShader.bindTexture("diffuseSampler", m_Texture, 0);

      m_VertexArray->bind();

      // TODO: Refactor these calls into the mesh or into a mesh renderer class maybe?
      //  1rst attribute buffer : vertices
      glEnableVertexAttribArray(0);
      m_VertexBuffer->bind();
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

      // 2nd attribute buffer : UV
      glEnableVertexAttribArray(1);
      m_UVBuffer->bind();
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

      // 3rd attribute buffer : normals
      glEnableVertexAttribArray(2);
      m_NormalBuffer->bind();
      glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

      m_IndexBuffer->bind();

      auto indices_count = int(m_Mesh->getIndices().size());
      GLCall(glDrawElements(GL_TRIANGLES, indices_count, GL_UNSIGNED_INT, nullptr));

      glDisableVertexAttribArray(0);
      glDisableVertexAttribArray(1);
      glDisableVertexAttribArray(2);
    }

    /// render to screen
    {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      auto windowWidth = int(ImGui::GetIO().DisplaySize.x);
      auto windowHeight = int(ImGui::GetIO().DisplaySize.y);

      // Render on the whole framebuffer, complete from the lower left corner to the upper right
      glViewport(0, 0, windowWidth, windowHeight);

      // Clear the screen
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Use RTT shader
      m_quadShader.bind();

      // Bind our texture in Texture Unit 0
      m_quadShader.bindTexture("renderedTexture", m_RenderedTexture, 0);
      // m_RenderedTexture->bind(0);
      // m_quadShader.setUniform1i("renderedTexture", 0);

      // glActiveTexture(GL_TEXTURE0);
      // glBindTexture(GL_TEXTURE_2D, m_RenderedTexture->getID());
      // Set our "renderedTexture" sampler to use Texture Unit 0
      // glUniform1i(texID, 0);

      // m_quadShader.setUniform1f("time", (float)(glfwGetTime() * 10.0f));
      // glUniform1f(timeID, (float)(glfwGetTime() * 10.0f));

      // 1rst attribute buffer : vertices
      glEnableVertexAttribArray(0);
      m_quadVertexBuffer->bind();
      // glBindBuffer(GL_ARRAY_BUFFER, m_quadVertexBuffer->getID());
      glVertexAttribPointer(0,  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                            3,  // size
                            GL_FLOAT,  // type
                            GL_FALSE,  // normalized?
                            0,         // stride
                            (void *)0  // array buffer offset
      );

      // Draw the triangles !
      glDrawArrays(GL_TRIANGLES, 0, 6);  // 2*3 indices starting at 0 -> 2 triangles

      glDisableVertexAttribArray(0);
    }
  }

  void onImGuiRender() override {
    ImGui::Begin("Quad Control Panel");

    ImGui::SliderFloat3("Light Position", glm::value_ptr(m_LightPos), -10.0f, 10.0f);
    ImGui::ColorEdit3("Light Color", glm::value_ptr(m_LightColor));
    ImGui::SliderFloat("Light Power", &m_LightPower, 0.0f, 100.0f);

    ImGui::Separator();
    ImGui::SliderFloat("Material Ambient", &m_AmbientStrength, 0.0f, 1.0f);
    ImGui::SliderFloat("Material Specular", &m_SpecularStrength, 0.0f, 10.0f);

    // TODO: put this in camera controller
    m_CameraController.onImGuiRender();

    ImGui::End();
  };

  void onWindowResize(int width, int height) override {
    m_CameraController.getProps().aspect = (float)width / (float)height;
  }

 private:
  float m_ClearColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};

  float m_RotationSpeed = 1.f;
  float m_Rotation = 0.0f;

  glm::vec3 m_LightPos = glm::vec3(4, 4, 4);
  glm::vec3 m_LightColor = glm::vec3(1, 1, 1);
  float m_LightPower = 50.f;

  float m_AmbientStrength = 0.1f;
  float m_SpecularStrength = 0.3f;

  CameraController m_CameraController;

  Shader m_standarShader;
  unique_ptr<Mesh> m_Mesh;
  unique_ptr<VertexArray> m_VertexArray;
  unique_ptr<VertexBuffer> m_VertexBuffer, m_NormalBuffer, m_UVBuffer;
  unique_ptr<IndexBuffer> m_IndexBuffer;
  unique_ptr<Texture> m_Texture;

  // TODO: create abstraction for Framebuffer, or the genral concept of RTT?
  bool m_RTTInitialized = false;
  Shader m_quadShader;
  GLuint m_offscreenBuffer = 0;
  unique_ptr<Texture> m_RenderedTexture;
  unique_ptr<VertexBuffer> m_quadVertexBuffer;
};