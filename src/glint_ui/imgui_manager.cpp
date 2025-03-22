#include "imgui_manager.h"

#include "renderer/render_pass.h"
#include "renderer/renderer.h"
#include "renderer/swapchain.h"
#include "renderer/vk_context.h"
#include "renderer/vk_utils.h"

namespace glint {

ImGuiManager::~ImGuiManager() { cleanup(); }

void ImGuiManager::init(Window* window, Renderer* renderer) {
  LOGFN;
  this->window = window;
  this->renderer = renderer;

  // Create descriptor pool for ImGui
  createDescriptorPool();

  // Initialize ImGui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  // Initialize ImGui GLFW adapter
  ImGui_ImplGlfw_InitForVulkan(window->getGLFWWindow(), true);

  // Initialize ImGui Vulkan adapter
  VkContext* context = renderer->getContext();
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = context->getInstance();
  init_info.PhysicalDevice = context->getPhysicalDevice();
  init_info.Device = context->getDevice();
  init_info.QueueFamily = context->getQueueFamilyIndices().graphicsFamily.value();
  init_info.Queue = context->getGraphicsQueue();
  init_info.PipelineCache = VK_NULL_HANDLE;
  init_info.DescriptorPool = imguiPool;
  init_info.Subpass = 0;
  init_info.MinImageCount = renderer->getSwapChain()->getImageCount();
  init_info.ImageCount = renderer->getSwapChain()->getImageCount();
  // init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init_info.MSAASamples = context->getMsaaSamples();
  init_info.Allocator = nullptr;
  init_info.CheckVkResultFn = nullptr;
  init_info.RenderPass = renderer->getRenderPass()->getRenderPass();

  // ImGui_ImplVulkan_Init(&init_info, renderer->getRenderPass()->getRenderPass());
  ImGui_ImplVulkan_Init(&init_info);

  // Upload fonts
  VkCommandBuffer commandBuffer = VkUtils::beginSingleTimeCommands();
  ImGui_ImplVulkan_CreateFontsTexture();
  VkUtils::endSingleTimeCommands(commandBuffer);
  // ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void ImGuiManager::newFrame() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void ImGuiManager::render(VkCommandBuffer commandBuffer) {
  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void ImGuiManager::cleanup() {
  LOGFN;
  if (renderer && imguiPool != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(renderer->getContext()->getDevice());
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    vkDestroyDescriptorPool(renderer->getContext()->getDevice(), imguiPool, nullptr);
    imguiPool = VK_NULL_HANDLE;
  }
}

void ImGuiManager::createDescriptorPool() {
  // VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
  //                                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
  //                                      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
  //                                      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
  //                                      {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
  //                                      {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
  //                                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
  //                                      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
  //                                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
  //                                      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
  //                                      {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

  // TODO: Add more descriptor types if needed, or reduce the count
  VkDescriptorPoolSize pool_sizes[] = {
      {VK_DESCRIPTOR_TYPE_SAMPLER, 100},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100},  // For images/textures
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100},          // For uniform data
  };

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  pool_info.maxSets = 100;
  pool_info.poolSizeCount = std::size(pool_sizes);
  pool_info.pPoolSizes = pool_sizes;

  if (vkCreateDescriptorPool(renderer->getContext()->getDevice(), &pool_info, nullptr, &imguiPool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create ImGui descriptor pool!");
  }
}

}  // namespace glint