#include "Core/SceneBase.h"
#include "Core/Shader.h"
#include "Core/VertexBuffer.h"

class QuadScene : public SceneBase {
 public:
  QuadScene() : SceneBase("Quad Scene") {}
  void onAttach() override {
    m_Shader.init("./src/shaders/quad.vert", "./src/shaders/quad.frag");

    // Quad Data
    // x, y, r, g, b
    float vertices[] = {
        -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  // Bottom-left (red)
        0.5f,  -0.5f, 0.0f, 1.0f, 0.0f,  // Bottom-right (green)
        0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  // Top-right (blue)
        -0.5f, 0.5f,  1.0f, 1.0f, 0.0f   // Top-left (yellow)
    };

    unsigned int indices[] = {0, 1, 2, 2, 3, 0};

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
  }

  void onDetach() override {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
  };

  void onUpdate(float deltaTime) override {
    // Rotate Quad
    m_Rotation += 0.05f * deltaTime;
    if (m_Rotation > 360.0f) m_Rotation -= 360.0f;
  };

  void onRender() override {
    // Clear screen
    glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // glm::mat4 transform = glm::mat4(1.0f);
    // transform = glm::rotate(transform, glm::radians(m_Rotation), glm::vec3(0.0f, 0.0f, 1.0f));

    // Render Quad
    // glUseProgram(shaderProgram);
    m_Shader.bind();
    // glUniformMatrix4fv(glGetUniformLocation(m_Shader.m_ID, "transform")) glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  };

  void onImGuiRender() override {
    ImGui::Begin("Quad Control Panel");
    ImGui::ColorEdit3("Quad Color", m_ClearColor);
    ImGui::Text("Press ESC to exit.");
    ImGui::End();
  };

 private:
  float m_ClearColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};
  float m_Rotation = 0.0f;

  Shader m_Shader;

  GLuint VBO, VAO, EBO;
};