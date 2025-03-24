
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "core/camera.h"
#include "core/config.h"
#include "core/logger.h"
#include "core/window.h"
#include "renderer/descriptor.h"
#include "renderer/mesh_factory.h"
#include "renderer/pipeline.h"
#include "renderer/render_pass.h"
#include "renderer/renderer.h"
#include "renderer/swapchain.h"
#include "renderer/texture.h"
#include "renderer/vk_utils.h"
#include "ui/imgui_manager.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

using namespace glint;

std::unordered_set<std::string> OneTimeLogger::loggedFunctions;

struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

class App {
 public:
  void run() {
    initWindow();
    initRenderer();
    initMesh();
    mainLoop();
    cleanup();
  }

 private:
  void initWindow() {
    Window::WindowProps props;
    props.title = "Glint - Triangle";
    props.width = WIDTH;
    props.height = HEIGHT;
    props.resizable = true;

    window = std::make_unique<Window>(props);
  }

  void initRenderer() {
    renderer = std::make_unique<Renderer>(window.get(), MAX_FRAMES_IN_FLIGHT);
    renderer->init();

    imguiManager = std::make_unique<ImGuiManager>();
    imguiManager->init(window.get(), renderer.get());

    // Create descriptor set layout
    m_DescriptorSetLayout = DescriptorSetLayout::Builder(renderer->getContext())
                                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                                .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                .build();

    // Verify the descriptor set layout is valid
    if (!m_DescriptorSetLayout || !m_DescriptorSetLayout->getLayout()) {
      throw std::runtime_error("Failed to create descriptor set layout");
    }

    // Create texture before descriptor updates
    m_Texture = std::make_unique<Texture>(renderer->getContext(), Config::getResourceFile("texture.jpg"));
    if (!m_Texture || !m_Texture->isValid()) {
      throw std::runtime_error("Failed to load texture");
    }

    PipelineConfig config;
    config.vertexShaderPath = Config::getShaderFile("basic_tex.vert");
    config.fragmentShaderPath = Config::getShaderFile("basic_tex.frag");
    config.vertexFormat = VertexAttributeFlags::POSITION_COLOR_TEXCOORD;
    config.depthTestEnable = true;
    config.depthWriteEnable = true;
    // config.cullMode = VK_CULL_MODE_BACK_BIT;
    config.cullMode = VK_CULL_MODE_NONE;
    config.descriptorSetLayout = m_DescriptorSetLayout.get();

    renderer->createPipeline(&config);

    uint32_t framesInFlight = renderer->getFramesInFlight();
    m_DescriptorPool =
        std::make_unique<DescriptorPool>(renderer->getContext(), m_DescriptorSetLayout.get(), framesInFlight);

    m_UniformBuffers.resize(framesInFlight);
    for (auto& ubo : m_UniformBuffers) {
      ubo = std::make_unique<UniformBuffer>(renderer->getContext(), sizeof(UniformBufferObject));
    }

    // Create descriptor
    m_Descriptor = std::make_unique<Descriptor>(renderer->getContext(), m_DescriptorSetLayout.get(),
                                                m_DescriptorPool.get(), framesInFlight);

    // Update descriptor with uniform buffer and texture
    for (uint32_t i = 0; i < framesInFlight; i++) {
      // Update uniform buffer
      m_Descriptor->updateUniformBuffer(0, m_UniformBuffers[i]->getBuffer(), sizeof(UniformBufferObject), 0, i);

      // Update texture sampler
      m_Descriptor->updateTextureSampler(1, m_Texture->getImageView(), m_Texture->getSampler(), i);
    }

    // Set initial transformation matrices
    VkExtent2D extent = renderer->getSwapChain()->getExtent();
    float aspect = extent.width / (float)extent.height;
  }

  void initMesh() {
    mesh = MeshFactory::createTexturedCube(renderer->getContext());
    // m_Texture = std::make_unique<Texture>(renderer->getContext(), "./res/texture.png");

    m_Camera.setPosition(0.0f, 0.0f, 2.0f);
    m_Camera.setTarget(0.0f, 0.0f, 0.0f);
  }

  void mainLoop() {
    auto lastFrameTime = std::chrono::high_resolution_clock::now();

    while (!window->shouldClose()) {
      window->pollEvents();

      processInput();

      // Calculate delta time
      auto currentTime = std::chrono::high_resolution_clock::now();
      float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastFrameTime).count();
      lastFrameTime = currentTime;

      update(deltaTime);

      renderer->drawFrame(
          [this](VkCommandBuffer commandBuffer, uint32_t imageIndex) { drawScene(commandBuffer, imageIndex); });
    }
    renderer->waitIdle();
  }

  virtual void processInput() {
    ImGuiIO& io = ImGui::GetIO();

    // Process mouse movement
    m_Camera.setScreenDimensions(window->getWidth(), window->getHeight());
    m_Camera.processMouseScroll(static_cast<int>(io.MouseWheel));
    m_Camera.processMouseMovement(static_cast<int>(io.MousePosPrev.x), static_cast<int>(io.MousePosPrev.y),
                                  static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y), io.MouseDown[1],
                                  io.MouseDown[2]);
  }

  void cleanup() {
    // cleanup will happen in correct order automatically!
  }

  void update(float deltaTime) {
    // Update rotation angle
    m_RotationAngle += deltaTime * 45.0f;  // 45 degrees per second

    // Keep angle between 0-360 degrees
    if (m_RotationAngle > 360.0f) {
      m_RotationAngle -= 360.0f;
    }

    static auto t = deltaTime;
    t += deltaTime;
    auto f = std::sin(t);
    m_ModelPosition = glm::vec3(f / 2, 0.0f, 0.0f);
    m_RotationAxis = glm::mix(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), f);

    m_Camera.update(deltaTime);

    // Get current frame index
    uint32_t currentFrame = renderer->getCurrentFrame();

    // Update the uniform buffer with new transformation
    updateUniformBuffer(currentFrame);
  }

  void updateUniformBuffer(uint32_t currentImage) {
    UniformBufferObject ubo{};

    // Create model matrix with rotation around z-axis
    ubo.model = glm::translate(glm::mat4(1.0f), m_ModelPosition);
    ubo.model = glm::rotate(ubo.model, glm::radians(m_RotationAngle), m_RotationAxis);

    ubo.view = m_Camera.getViewMatrix();
    ubo.proj = m_Camera.getProjectionMatrix();

    // Flip Y coordinate for Vulkan
    ubo.proj[1][1] *= -1;

    // Update the uniform buffer
    m_UniformBuffers[currentImage]->update(&ubo, sizeof(ubo));
  }

  std::unique_ptr<Window> window = nullptr;
  std::unique_ptr<Renderer> renderer = nullptr;
  std::unique_ptr<Mesh> mesh = nullptr;
  std::unique_ptr<Texture> m_Texture = nullptr;
  glint::Camera m_Camera;

  std::unique_ptr<DescriptorSetLayout> m_DescriptorSetLayout = nullptr;
  std::unique_ptr<DescriptorPool> m_DescriptorPool = nullptr;
  std::unique_ptr<Descriptor> m_Descriptor = nullptr;
  std::vector<std::unique_ptr<UniformBuffer>> m_UniformBuffers;

  std::unique_ptr<glint::ImGuiManager> imguiManager = nullptr;

  float m_RotationAngle = 0.0f;
  glm::vec3 m_ModelPosition = glm::vec3(0.0f);
  glm::vec3 m_RotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);

  void drawScene(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    // Get components from renderer
    auto renderPass = renderer->getRenderPass();
    auto pipeline = renderer->getPipeline();
    auto swapChain = renderer->getSwapChain();

    glint::ImGuiManager::newFrame();

    renderPass->begin(commandBuffer, imageIndex, {0.0f, 0.0f, 0.0f, 1.0f});
    pipeline->bind(commandBuffer);

    m_Descriptor->bind(commandBuffer, pipeline->getPipelineLayout(), renderer->getCurrentFrame());

    mesh->bind(commandBuffer);

    ImGuiIO& io = ImGui::GetIO();
    ImGui::Begin("Glint Stats");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
    ImGui::Text("Position: %.2f, %.2f, %.2f", m_Camera.getPosition().x, m_Camera.getPosition().y,
                m_Camera.getPosition().z);
    ImGui::Text("Mouse wheel delta %.2f", io.MouseWheel);
    ImGui::Text("Mouse position %.2f, %.2f", io.MousePos.x, io.MousePos.y);
    ImGui::End();

    auto swapChainExtent = swapChain->getExtent();
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    mesh->draw(commandBuffer);

    glint::ImGuiManager::render(commandBuffer);

    renderPass->end(commandBuffer);
  }
};

int main(int argc, char** argv) {
  Config::initialize(argc, argv);

  App app;
  try {
    app.run();
  } catch (const std::exception& e) {
    LOG("[ERROR]", __FUNCTION__, e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
