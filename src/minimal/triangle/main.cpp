
#include "core/config.h"
#include "core/logger.h"
#include "core/window.h"
#include "renderer/mesh_factory.h"
#include "renderer/pipeline.h"
#include "renderer/render_pass.h"
#include "renderer/renderer.h"
#include "renderer/swapchain.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

std::unordered_set<std::string> glint::OneTimeLogger::loggedFunctions;

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
    glint::Window::WindowProps props;
    props.title = "Glint - Triangle";
    props.width = WIDTH;
    props.height = HEIGHT;
    props.resizable = true;

    window = std::make_unique<glint::Window>(props);
  }

  void initRenderer() {
    renderer = std::make_unique<glint::Renderer>(window.get(), MAX_FRAMES_IN_FLIGHT);
    renderer->init();

    glint::PipelineConfig config;
    config.vertexShaderPath = glint::Config::getShaderFile("basic.vert");
    config.fragmentShaderPath = glint::Config::getShaderFile("shader.frag");
    config.descriptorSetLayout = nullptr;
    config.vertexFormat = glint::VertexAttributeFlags::POSITION_COLOR;
    renderer->createPipeline(&config);
  }

  void initMesh() { mesh = glint::MeshFactory::createTriangle(renderer->getContext()); }

  void mainLoop() {
    while (!window->shouldClose()) {
      window->pollEvents();
      renderer->drawFrame(
          [this](VkCommandBuffer commandBuffer, uint32_t imageIndex) { drawScene(commandBuffer, imageIndex); });
    }
    renderer->waitIdle();
  }

  void cleanup() {
    // cleanup will happen in correct order automatically!
  }

  std::unique_ptr<glint::Window> window = nullptr;
  std::unique_ptr<glint::Renderer> renderer = nullptr;
  std::unique_ptr<glint::Mesh> mesh = nullptr;

  void drawScene(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    // Get components from renderer
    auto renderPass = renderer->getRenderPass();
    auto pipeline = renderer->getPipeline();
    auto swapChain = renderer->getSwapChain();

    renderPass->begin(commandBuffer, imageIndex, {0.0f, 0.0f, 0.0f, 1.0f});
    pipeline->bind(commandBuffer);

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
  glint::Config::initialize(argc, argv);

  App app;
  try {
    app.run();
  } catch (const std::exception& e) {
    LOG("[ERROR]", __FUNCTION__, e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
