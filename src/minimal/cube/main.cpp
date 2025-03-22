
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "core/config.h"
#include "core/logger.h"
#include "core/utils.h"
#include "core/window.h"
#include "renderer/descriptor.h"
#include "renderer/mesh_factory.h"
#include "renderer/pipeline.h"
#include "renderer/render_pass.h"
#include "renderer/renderer.h"
#include "renderer/swapchain.h"
#include "renderer/texture.h"
#include "renderer/ubo_data.h"
#include "renderer/vk_utils.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

using namespace glint;

std::unordered_set<std::string> OneTimeLogger::loggedFunctions;

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
    m_Texture = std::make_unique<Texture>(renderer->getContext(), "./res/texture.jpg");
    if (!m_Texture || !m_Texture->isValid()) {
      throw std::runtime_error("Failed to load texture");
    }

    PipelineConfig config;
    config.vertexShaderPath = getShaderPath("basic_tex.vert");
    config.fragmentShaderPath = getShaderPath("basic_tex.frag");
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
      m_Descriptor->updateUniformBuffer(m_UniformBuffers[i]->getBuffer(), sizeof(UniformBufferObject), 0, i);

      // Update texture sampler
      m_Descriptor->updateTextureSampler(m_Texture->getImageView(), m_Texture->getSampler(), i);
    }

    // Set initial transformation matrices
    VkExtent2D extent = renderer->getSwapChain()->getExtent();
    float aspect = extent.width / (float)extent.height;
  }

  void initMesh() {
    mesh = MeshFactory::createTexturedCube(renderer->getContext());
    // m_Texture = std::make_unique<Texture>(renderer->getContext(), "./res/texture.png");
  }

  void mainLoop() {
    auto lastFrameTime = std::chrono::high_resolution_clock::now();

    while (!window->shouldClose()) {
      window->pollEvents();

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

    // View matrix - slight distance from the quad
    ubo.view = glm::lookAt(glm::vec3(2.0f, 0.2f, 2.0f),   // Camera position
                           glm::vec3(0.0f, 0.0f, 0.0f),   // Look at origin
                           glm::vec3(0.0f, 1.0f, 0.0f));  // Up vector

    // Projection matrix
    VkExtent2D extent = renderer->getSwapChain()->getExtent();
    float aspect = extent.width / (float)extent.height;
    ubo.proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

    // Flip Y coordinate for Vulkan
    ubo.proj[1][1] *= -1;

    // Update the uniform buffer
    m_UniformBuffers[currentImage]->update(&ubo, sizeof(ubo));
  }

  std::unique_ptr<Window> window = nullptr;
  std::unique_ptr<Renderer> renderer = nullptr;
  std::unique_ptr<Mesh> mesh = nullptr;
  std::unique_ptr<Texture> m_Texture = nullptr;

  std::unique_ptr<DescriptorSetLayout> m_DescriptorSetLayout = nullptr;
  std::unique_ptr<DescriptorPool> m_DescriptorPool = nullptr;
  std::unique_ptr<Descriptor> m_Descriptor = nullptr;
  std::vector<std::unique_ptr<UniformBuffer>> m_UniformBuffers;

  float m_RotationAngle = 0.0f;
  glm::vec3 m_ModelPosition = glm::vec3(0.0f);
  glm::vec3 m_RotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);

  void drawScene(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    // Get components from renderer
    auto renderPass = renderer->getRenderPass();
    auto pipeline = renderer->getPipeline();
    auto swapChain = renderer->getSwapChain();

    renderPass->begin(commandBuffer, imageIndex, {0.0f, 0.0f, 0.0f, 1.0f});
    pipeline->bind(commandBuffer);

    m_Descriptor->bind(commandBuffer, pipeline->getPipelineLayout(), renderer->getCurrentFrame());

    mesh->bind(commandBuffer);

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
