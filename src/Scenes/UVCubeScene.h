#include <memory>

// core
#include "Core/IndexBuffer.h"
#include "Core/SceneBase.h"
#include "Core/Shader.h"
#include "Core/Texture.h"
#include "Core/VertexArray.h"
#include "Core/VertexBuffer.h"

// glm
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class UVCubeScene : public SceneBase {
 public:
  UVCubeScene() : SceneBase("UV Cube Scene") {}
  void onAttach() override {
    m_Shader.init(getFilePath("/shaders/simple_uv.vert"), getFilePath("/shaders/simple_uv.frag"));

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it is closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cube Data
    // Our vertices. Tree consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
    // A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
    static const GLfloat vertex_buffer_data[] = {
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
        1.0f,  -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  -1.0f,
        1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, -1.0f, 1.0f, -1.0f, 1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,
        -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f, 1.0f,  -1.0f, 1.0f};

    // Two UV coordinatesfor each vertex. They were created with Blender.
    static const GLfloat g_uv_buffer_data[] = {
        0.000059f, 1.0f - 0.000004f, 0.000103f, 1.0f - 0.336048f, 0.335973f, 1.0f - 0.335903f,
        1.000023f, 1.0f - 0.000013f, 0.667979f, 1.0f - 0.335851f, 0.999958f, 1.0f - 0.336064f,
        0.667979f, 1.0f - 0.335851f, 0.336024f, 1.0f - 0.671877f, 0.667969f, 1.0f - 0.671889f,
        1.000023f, 1.0f - 0.000013f, 0.668104f, 1.0f - 0.000013f, 0.667979f, 1.0f - 0.335851f,
        0.000059f, 1.0f - 0.000004f, 0.335973f, 1.0f - 0.335903f, 0.336098f, 1.0f - 0.000071f,
        0.667979f, 1.0f - 0.335851f, 0.335973f, 1.0f - 0.335903f, 0.336024f, 1.0f - 0.671877f,
        1.000004f, 1.0f - 0.671847f, 0.999958f, 1.0f - 0.336064f, 0.667979f, 1.0f - 0.335851f,
        0.668104f, 1.0f - 0.000013f, 0.335973f, 1.0f - 0.335903f, 0.667979f, 1.0f - 0.335851f,
        0.335973f, 1.0f - 0.335903f, 0.668104f, 1.0f - 0.000013f, 0.336098f, 1.0f - 0.000071f,
        0.000103f, 1.0f - 0.336048f, 0.000004f, 1.0f - 0.671870f, 0.336024f, 1.0f - 0.671877f,
        0.000103f, 1.0f - 0.336048f, 0.336024f, 1.0f - 0.671877f, 0.335973f, 1.0f - 0.335903f,
        0.667969f, 1.0f - 0.671889f, 1.000004f, 1.0f - 0.671847f, 0.667979f, 1.0f - 0.335851f};

    // unsigned int indices[] = {0, 1, 2, 2, 3, 0};

    // order matters here
    m_VertexArray = std::make_unique<VertexArray>();

    m_VertexBuffer = std::make_unique<VertexBuffer>(vertex_buffer_data, sizeof(vertex_buffer_data));
    m_UVBuffer = std::make_unique<VertexBuffer>(g_uv_buffer_data, sizeof(g_uv_buffer_data));
    // m_Texture = std::make_unique<Texture>(getFilePath("/res/textures/box.jpg"));
    m_Texture = std::make_unique<Texture>(getFilePath("/res/textures/grid.png"));

    // m_IndexBuffer = std::make_unique<IndexBuffer>(indices, 6);

    // Position attribute
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0));
    GLCall(glEnableVertexAttribArray(0));

    // Color attribute
    GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(2 * sizeof(float))));
    GLCall(glEnableVertexAttribArray(1));
  }

  void onDetach() override {
    m_VertexBuffer.reset();
    // m_IndexBuffer.reset();
    m_VertexArray.reset();
  };

  void onUpdate(float deltaTime) override {
    // Rotate Quad
    m_Rotation += m_RotationSpeed * deltaTime * 10;
    if (m_Rotation > 360.0f) m_Rotation -= 360.0f;
  };

  glm::mat4 getViewMatrix() {
    // Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    // Camera matrix
    glm::mat4 View = glm::lookAt(glm::vec3(4, 3, -3),  // Camera is at (4,3,-3), in World Space
                                 glm::vec3(0, 0, 0),   // and looks at the origin
                                 glm::vec3(0, 1, 0)    // Head is up (set to 0,-1,0 to look upside-down)
    );
    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0.5f, 1.0f, 0.0f));
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 MVP = Projection * View * Model;  // Remember, matrix multiplication is the other way around
    return MVP;
  }

  void onRender() override {
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

    // 2nd attribute buffer : colors
    glEnableVertexAttribArray(1);
    // glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    // m_ColorBuffer->bind();
    glVertexAttribPointer(1,         // attribute. No particular reason for 1, but must match the layout in the shader.
                          3,         // size
                          GL_FLOAT,  // type
                          GL_FALSE,  // normalized?
                          0,         // stride
                          (void *)0  // array buffer offset
    );

    // GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
    GLCall(glDrawArrays(GL_TRIANGLES, 0, 12 * 3));

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
  };

  void onImGuiRender() override {
    ImGui::Begin("Quad Control Panel");
    ImGui::ColorEdit3("Quad Color", m_ClearColor);
    ImGui::SliderFloat("Rotation Speed", &m_RotationSpeed, -10.0f, 10.0f);
    ImGui::Text("Press ESC to exit.");
    ImGui::End();
  };

 private:
  float m_ClearColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};
  float m_RotationSpeed = 1.f;
  float m_Rotation = 0.0f;

  Shader m_Shader;

  std::unique_ptr<VertexBuffer> m_VertexBuffer, m_UVBuffer;
  std::unique_ptr<VertexArray> m_VertexArray;
  std::unique_ptr<Texture> m_Texture;
};