#pragma once

// core
#include "Core/SceneBase.h"
#include "Graphics/Camera.h"
#include "Graphics/FrameBuffer.h"
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

class ShadowMapping : public SceneBase {
 public:
  ShadowMapping() : SceneBase("Shadow Mapping"), m_CameraController(getDefaultCameraProps()) {
    m_CameraController.getProps().position = glm::vec3(5, 4, -13);
  }
  void onAttach() override {
    Logger::log("ShadowMapping::onAttach()\n\n");

    glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.0f);

    m_depthShader.init(getFilePath("/shaders/depth_rtt.vert"), getFilePath("/shaders/depth_rtt.frag"));
    m_shadowMapping.init(getFilePath("/shaders/shadow_mapping.vert"), getFilePath("/shaders/shadow_mapping.frag"));

    Renderer::setup3D();

    m_Mesh = std::make_unique<Mesh>(getFilePath("/res/room/room.obj"));
    m_Mesh->index();

    m_VertexArray = std::make_unique<VertexArray>();

    const auto &vertices = m_Mesh->getVertices();
    const auto &indices = m_Mesh->getIndices();
    const auto &tex_coords = m_Mesh->getTexCoords();
    const auto &normals = m_Mesh->getNormals();

    m_VertexBuffer = std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(glm::vec3));
    m_NormalBuffer = std::make_unique<VertexBuffer>(normals.data(), normals.size() * sizeof(glm::vec3));
    m_UVBuffer = std::make_unique<VertexBuffer>(tex_coords.data(), tex_coords.size() * sizeof(glm::vec2));
    m_IndexBuffer = std::make_unique<IndexBuffer>(indices.data(), int(indices.size()));

    m_Texture = std::make_unique<Texture>(getFilePath("/res/room/uvmap.jpg"));

    /// Setup RTT
    setupRtt();
  }

  void setupRtt() {
    if (m_RTTInitialized) return;

    auto windowWidth = int(ImGui::GetIO().DisplaySize.x);
    auto windowHeight = int(ImGui::GetIO().DisplaySize.y);
    Logger::log("Window Width: ", windowWidth, " Window Height: ", windowHeight);
    if (windowWidth < 0 || windowHeight < 0) {
      Logger::error("Window width or height is not set yet. Will try again later.");
      return;
    }

    m_offscreenBuffer.create(windowWidth, windowHeight);
    // generate and bind texture we are going to render to
    // this now also sets the texture as the currentcolor attachment #0
    // m_offscreenBuffer.createColorAttachment();
    // and the depth buffer
    m_offscreenBuffer.createDepthAttachment(FrameBuffer::DepthAttachmentType::Texture);

    // No color output in the bound framebuffer, only depth.
    // TODO: Move this into FrameBuffer class?
    // check if this is needed
    glDrawBuffer(GL_NONE);

    // Always check that our framebuffer is ok
    // if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    //   Logger::error("Framebuffer not complete!");
    //   return;
    //   // return false
    // }

    // The fullscreen quad's FBO
    static const GLfloat g_quad_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f,  0.0f, 1.0f, -1.0f, 0.0f, 1.0f,  1.0f, 0.0f,
    };

    m_quadVertexBuffer = make_unique<VertexBuffer>(g_quad_vertex_buffer_data, sizeof(g_quad_vertex_buffer_data));
    // Create and compile our GLSL program from the shaders
    m_quadShader.init(getFilePath("/shaders/passthrough.vert"), getFilePath("/shaders/SimpleTexture.frag"));

    m_RTTInitialized = true;
  }

  void onDetach() override { glDisable(GL_CULL_FACE); };

  void onUpdate(float deltaTime) override {
    // Update camera
    m_CameraController.update(deltaTime);
  };

  void renderDepth() {
    m_offscreenBuffer.bind();
    glViewport(0, 0, int(ImGui::GetIO().DisplaySize.x), int(ImGui::GetIO().DisplaySize.y));

    // We don't use bias in the shader, but instead we draw back faces,
    // which are already separated from the front faces by a small distance
    // (if your geometry is made this way)
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);  // Cull back-facing triangles -> draw only front-facing triangles

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    m_depthShader.bind();

    // glm::vec3 lightInvDir = glm::vec3(0.5f, 2, 2);
    glm::vec3 lightInvDir = -m_LightPos;
    // glm::vec3 lightInvDir = m_LightPos;

    // Compute the MVP matrix from the light's point of view
    glm::mat4 depthProjectionMatrix = glm::ortho<float>(-10, 10, -10, 10, -10, 20);
    glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    // or, for spot light :
    // glm::vec3 lightPos(5, 20, 20);
    // glm::mat4 depthProjectionMatrix = glm::perspective<float>(45.0f, 1.0f, 2.0f, 50.0f);
    // glm::mat4 depthViewMatrix = glm::lookAt(lightPos, lightPos-lightInvDir, glm::vec3(0,1,0));

    glm::mat4 depthModelMatrix = glm::mat4(1.0);
    m_depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    m_depthShader.setUniformMat4("depthMVP", m_depthMVP);

    // m_VertexArray->bind();

    //  1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    m_VertexBuffer->bind();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    m_IndexBuffer->bind();

    GLCall(glDrawElements(GL_TRIANGLES, int(m_Mesh->getIndices().size()), GL_UNSIGNED_INT, nullptr));

    glEnableVertexAttribArray(0);
  }

  void renderToScreen() {
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_offscreenBuffer.unbind();

    // Render on the whole framebuffer, complete from the lower left corner to the upper right
    auto windowWidth = int(ImGui::GetIO().DisplaySize.x);
    auto windowHeight = int(ImGui::GetIO().DisplaySize.y);
    glViewport(0, 0, windowWidth, windowHeight);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);  // Cull back-facing triangles -> draw only front-facing triangles

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shadowMapping.bind();

    glm::mat4 Projection = m_CameraController.getProjectionMatrix();
    glm::mat4 View = m_CameraController.getViewMatrix();
    glm::mat4 Model = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0.5f, 1.0f, 0.0f));
    glm::mat4 MVP = Projection * View * Model;

    glm::mat4 biasMatrix(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0);

    glm::mat4 depthBiasMVP = biasMatrix * m_depthMVP;

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    // glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    // glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
    // glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
    // glUniformMatrix4fv(DepthBiasID, 1, GL_FALSE, &depthBiasMVP[0][0]);

    // glUniform3f(lightInvDirID, lightInvDir.x, lightInvDir.y, lightInvDir.z);

    // Camera
    m_shadowMapping.setUniformMat4("MVP", MVP);
    m_shadowMapping.setUniformMat4("V", View);
    m_shadowMapping.setUniformMat4("M", Model);
    m_shadowMapping.setUniformMat4("DepthBiasMVP", depthBiasMVP);

    // glm::vec3 lightInvDir = glm::vec3(0.5f, 2, 2);
    glm::vec3 lightInvDir = -m_LightPos;
    // glm::vec3 lightInvDir = m_LightPos;

    m_shadowMapping.setUniform3f("LightInvDirection_worldspace", lightInvDir.x, lightInvDir.y, lightInvDir.z);

    // Bind our texture in Texture Unit 0
    m_shadowMapping.bindTexture("myTextureSampler", m_Texture, 0);
    //   glActiveTexture(GL_TEXTURE0);
    //   glBindTexture(GL_TEXTURE_2D, Texture);
    // Set our "myTextureSampler" sampler to use Texture Unit 0
    //   glUniform1i(TextureID, 0);

    //   m_shadowMapping.bindTexture("shadowMap", m_offscreenBuffer.getDepthRenderBuffer(), 1);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, m_offscreenBuffer.getDepthRenderBuffer());
    m_shadowMapping.setUniform1i("shadowMap", 1);

    // Lights
    m_shadowMapping.setUniform3f("LightPosition_worldspace", m_LightPos.x, m_LightPos.y, m_LightPos.z);
    m_shadowMapping.setUniform3f("LightColor", m_LightColor.x, m_LightColor.y, m_LightColor.z);
    m_shadowMapping.setUniform1f("LightPower", m_LightPower);

    // Material
    m_shadowMapping.setUniform1f("MaterialAmbient", m_AmbientStrength);
    m_shadowMapping.setUniform1f("MaterialSpecular", m_SpecularStrength);

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

  void onRender() override {
    if (!m_RTTInitialized) {
      Logger::error("RTT not initialized yet. Retrying...");
      setupRtt();
      return;
    }
    renderDepth();
    renderToScreen();
  }

  void onImGuiRender() override {
    ImGui::Begin("RTT Panel");

    ImGui::SliderFloat3("Light Position", glm::value_ptr(m_LightPos), -10.0f, 10.0f);
    ImGui::ColorEdit3("Light Color", glm::value_ptr(m_LightColor));
    ImGui::SliderFloat("Light Power", &m_LightPower, 0.0f, 100.0f);

    ImGui::Separator();
    ImGui::SliderFloat("Material Ambient", &m_AmbientStrength, 0.0f, 1.0f);
    ImGui::SliderFloat("Material Specular", &m_SpecularStrength, 0.0f, 10.0f);

    ImGui::Separator();
    ImGui::Checkbox("Show Depth Buffer", &m_ShowDepthBuffer);

    if (m_ShowDepthBuffer) {
      ImGui::SliderFloat("Depth Near", &m_depthNear, 0.001f, 0.1f);
      ImGui::SliderFloat("Depth Far", &m_depthFar, 0.1f, 10.0f);
      ImGui::SliderFloat("Depth Scale", &m_depthScale, 0.1f, 100.0f);

      ImGui::Image(m_offscreenBuffer.getDepthRenderBuffer(),
                   ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2), {0, 1}, {1, 0});
    }

    ImGui::End();

    m_CameraController.onImGuiRender();
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

  // used to visualize depth
  bool m_ShowDepthBuffer = false;
  float m_depthNear = 0.1f;
  float m_depthFar = 100.0f;
  float m_depthScale = 1.0f;

  CameraController m_CameraController;

  Shader m_depthShader;
  unique_ptr<Mesh> m_Mesh;
  unique_ptr<VertexArray> m_VertexArray;
  unique_ptr<VertexBuffer> m_VertexBuffer, m_NormalBuffer, m_UVBuffer;
  unique_ptr<IndexBuffer> m_IndexBuffer;
  unique_ptr<Texture> m_Texture;

  // TODO: create abstraction for Framebuffer, or the genral concept of RTT?
  bool m_RTTInitialized = false;
  Shader m_quadShader;
  FrameBuffer m_offscreenBuffer;
  unique_ptr<VertexBuffer> m_quadVertexBuffer;

  Shader m_shadowMapping;
  glm::mat4 m_depthMVP;
};