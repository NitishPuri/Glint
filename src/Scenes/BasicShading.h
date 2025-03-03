#include <memory>

// core
#include "Core/Camera.h"
#include "Core/IndexBuffer.h"
#include "Core/Mesh.h"
#include "Core/SceneBase.h"
#include "Core/Shader.h"
#include "Core/Texture.h"
#include "Core/VertexArray.h"
#include "Core/VertexBuffer.h"

// glm
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class BasicShading : public SceneBase {
 public:
  BasicShading() : SceneBase("Basic Shading and Model Loading"), m_CameraController(getDefaultCameraProps()) {}
  void onAttach() override {
    m_Shader.init(getFilePath("/shaders/simple_uv.vert"), getFilePath("/shaders/simple_uv.frag"));

    // TODO: Move to something like Renderer::setup3D() ?
    //  Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it is closer to the camera than the former one
    glDepthFunc(GL_LESS);

    m_Mesh = std::make_unique<Mesh>(getFilePath("/res/cube.obj"));

    // order matters here
    m_VertexArray = std::make_unique<VertexArray>();

    const auto &vertices = m_Mesh->getVertices();
    const auto &indices = m_Mesh->getIndices();
    const auto &tex_coords = m_Mesh->getTexCoords();

    m_VertexBuffer = std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(float));
    m_IndexBuffer = std::make_unique<IndexBuffer>(indices.data(), indices.size());
    m_UVBuffer = std::make_unique<VertexBuffer>(tex_coords.data(), tex_coords.size() * sizeof(float));

    m_Texture = std::make_unique<Texture>(getFilePath("/res/textures/box.jpg"));
  }

  void onDetach() override {};

  void onUpdate(float deltaTime) override {
    // Rotate Quad
    m_Rotation += m_RotationSpeed * deltaTime * 10;
    if (m_Rotation > 360.0f) m_Rotation -= 360.0f;

    // Update camera
    m_CameraController.update(deltaTime);
  };

  glm::mat4 getViewMatrix() {
    auto ViewProjection = m_CameraController.getViewProjection();

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0.5f, 1.0f, 0.0f));

    glm::mat4 MVP = ViewProjection * Model;  // Remember, matrix multiplication is the other way around
    return MVP;
  }

  void onRender() override {
    Logger::log("BasicShading::onRender");
    // Clear screen
    GLCall(glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    glm::mat4 transform = getViewMatrix();

    // Render Quad
    m_Shader.bind();
    m_Shader.setUniformMat4("MVP", transform);

    m_VertexArray->bind();

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    m_VertexBuffer->bind();
    glVertexAttribPointer(0,         // attribute. No particular reason for 0, but must match the layout in the shader.
                          3,         // size
                          GL_FLOAT,  // type
                          GL_FALSE,  // normalized?
                          0,         // stride
                          (void *)0  // array buffer offset
    );

    // 2nd attribute buffer : UV
    glEnableVertexAttribArray(1);
    m_UVBuffer->bind();
    glVertexAttribPointer(1,         // attribute. No particular reason for 1, but must match the layout in the shader.
                          2,         // size
                          GL_FLOAT,  // type
                          GL_FALSE,  // normalized?
                          0,         // stride
                          (void *)0  // array buffer offset
    );

    GLCall(glDrawArrays(GL_TRIANGLES, 0, 12 * 3));

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
  };

  void onImGuiRender() override {
    ImGui::Begin("Quad Control Panel");

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

  int m_currentTexture = 0;

  Shader m_Shader;
  CameraController m_CameraController;

  std::unique_ptr<Mesh> m_Mesh;

  std::unique_ptr<VertexArray> m_VertexArray;
  std::unique_ptr<VertexBuffer> m_VertexBuffer;
  std::unique_ptr<VertexBuffer> m_UVBuffer;
  std::unique_ptr<IndexBuffer> m_IndexBuffer;
  std::unique_ptr<Texture> m_Texture;
};