
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

#include "core/window.h"
#include "logger.h"
#include "renderer/swapchain.h"
#include "renderer/vulkan_context.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

std::unordered_set<std::string> glint::OneTimeLogger::loggedFunctions;

static std::vector<char> readFile(const std::string& filename) {
  LOGFN;
  std::ifstream file{filename, std::ios::ate | std::ios::binary};

  if (!file.is_open()) {
    throw std::runtime_error("failed to open file!");
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  LOG("Loading filename:", filename, "fileSize:", fileSize, "bytes");

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();
  return buffer;
}

class App {
 public:
#pragma region APP
  void run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
  }

 private:
  void initWindow() {
    LOGFN;
    glint::Window::WindowProps props;
    props.title = "Vulkan";
    props.width = WIDTH;
    props.height = HEIGHT;
    props.resizable = false;

    window = std::make_unique<glint::Window>(props);
  }

  void initVulkan() {
    LOGFN;

    // Create Vulkan Context
    vulkanContext = std::make_unique<glint::VulkanContext>(window.get());
    vulkanContext->init();

    // Get device from context
    device = vulkanContext->getDevice();
    graphicsQueue = vulkanContext->getGraphicsQueue();
    presentQueue = vulkanContext->getPresentQueue();
    // physicalDevice = vulkanContext->getPhysicalDevice();
    surface = vulkanContext->getSurface();

    // Create SwapChain
    swapChain = std::make_unique<glint::SwapChain>(vulkanContext.get());
    // createSwapChain();
    // createImageViews();
    swapChainImageFormat = swapChain->getImageFormat();
    swapChainExtent = swapChain->getExtent();

    createRenderPass();
    createGraphicsPipeline();

    // Create Framebuffers
    swapChain->createFramebuffers(renderPass);

    // createSwapChainBuffers();
    createCommandPool();
    createCommandBuffer();
    createSyncObjects();
  }

  void mainLoop() {
    LOGFN;

    static int frames = 0;
    // LOGCALL(while (!glfwWindowShouldClose(window))) {
    LOGCALL(while (!window->shouldClose())) {
      glfwPollEvents();
      drawFrame();

      frames++;
      // if (frames == 2) break;
    }

    LOGCALL(vkDeviceWaitIdle(device));
  }

  void cleanup() {
    LOGFN;

    LOGCALL(vkDestroySemaphore(device, renderFinishedSemaphore, nullptr));
    LOGCALL(vkDestroySemaphore(device, imageAvailableSemaphore, nullptr));
    LOGCALL(vkDestroyFence(device, inFlightFence, nullptr));

    LOGCALL(vkDestroyCommandPool(device, commandPool, nullptr));

    // for (auto framebuffer : swapChainFrameBuffers) {
    //   LOGCALL(vkDestroyFramebuffer(device, framebuffer, nullptr));
    // }

    LOGCALL(vkDestroyPipeline(device, graphicsPipeline, nullptr));
    LOGCALL(vkDestroyPipelineLayout(device, pipelineLayout, nullptr));
    LOGCALL(vkDestroyRenderPass(device, renderPass, nullptr));

    // for (auto imageView : swapChainImageViews) {
    //   LOGCALL(vkDestroyImageView(device, imageView, nullptr));
    // }

    // LOGCALL(vkDestroySwapchainKHR(device, swapChain, nullptr));

    swapChain.reset();
    // vulkanContext->cleanup();
    vulkanContext.reset();
    window.reset();
  }

#pragma endregion APP

#pragma region VARIABLES
  std::unique_ptr<glint::Window> window = nullptr;
  std::unique_ptr<glint::VulkanContext> vulkanContext = nullptr;
  std::unique_ptr<glint::SwapChain> swapChain = nullptr;

  // VkInstance instance{};
  // VkDebugUtilsMessengerEXT debugMessenger;

  // VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

  VkDevice device;
  VkQueue graphicsQueue;

  VkSurfaceKHR surface;
  VkQueue presentQueue;

  // VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;

  // std::vector<VkImageView> swapChainImageViews;

  VkRenderPass renderPass;
  VkPipelineLayout pipelineLayout;

  VkPipeline graphicsPipeline;

  // std::vector<VkFramebuffer> swapChainFrameBuffers;

  VkCommandPool commandPool;
  VkCommandBuffer commandBuffer;

  VkSemaphore imageAvailableSemaphore;
  VkSemaphore renderFinishedSemaphore;
  VkFence inFlightFence;

#pragma endregion VARIABLES

#pragma region PIPELINE
  void createGraphicsPipeline() {
    LOGFN;

    LOG("Loading shaders, can wither load pre compiled shaders, or compile at runtime to SPIR-V");
    auto vertShaderCode = readFile("./bin/shaders/shader.vert.spv");
    auto fragShaderCode = readFile("./bin/shaders/shader.frag.spv");

    LOGCALL(VkShaderModule vertShaderModule = createShaderModule(vertShaderCode));
    LOGCALL(VkShaderModule fragShaderModule = createShaderModule(fragShaderCode));

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";  // entry point, whihch means you can control the entry point in the shader
    // vertShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";  // entry point, whihch means you can control the entry point in the shader
    // fragShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    LOG("Vertex Input");
    LOGCALL(VkPipelineVertexInputStateCreateInfo vertexInputInfo{});
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;  // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;  // Optional

    LOG("Input Assembly");
    LOGCALL(VkPipelineInputAssemblyStateCreateInfo inputAssembly{});
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;  //
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewports and scissors - if dynamic states are not used
    // LOGCALL(VkViewport viewport{});
    // viewport.x = 0.0f;
    // viewport.y = 0.0f;
    // viewport.width = (float)swapChainExtent.width;
    // viewport.height = (float)swapChainExtent.height;
    // viewport.minDepth = 0.0f;
    // viewport.maxDepth = 1.0f;

    // VkRect2D scissor{};
    // scissor.offset = {0, 0};
    // scissor.extent = swapChainExtent;

    LOG("Viewport State, dynamic states are used for viewport and scissor");
    LOGCALL(VkPipelineViewportStateCreateInfo viewportState{});
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = nullptr;  // dynamic states
    viewportState.scissorCount = 1;
    viewportState.pScissors = nullptr;  // dynamic states

    // Rasterizer
    LOG("Rasterizer");
    LOGCALL(VkPipelineRasterizationStateCreateInfo rasterizer{});
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    LOG("Using anything else requires enabling GPU features.");
    LOGCALL(rasterizer.polygonMode = VK_POLYGON_MODE_FILL);  // VK_POLYGON_MODE_LINE, VK_POLYGON_MODE_POINT
    LOGCALL(rasterizer.lineWidth = 1.0f);
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;     // VK_CULL_MODE_FRONT_BIT, VK_CULL_MODE_FRONT_AND_BACK
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;  // VK_FRONT_FACE_COUNTER_CLOCKWISE
    LOGCALL(rasterizer.depthBiasEnable = VK_FALSE);
    LOG("Depth Bias, for shadow mapping.");
    rasterizer.depthBiasConstantFactor = 0.0f;  // Optional
    rasterizer.depthBiasClamp = 0.0f;           // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f;     // Optional

    // Multisampling
    LOG("Multisampling, disabled for now.");
    LOGCALL(VkPipelineMultisampleStateCreateInfo multisampling{});
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;           // Optional
    multisampling.pSampleMask = nullptr;             // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE;  // Optional
    multisampling.alphaToOneEnable = VK_FALSE;       // Optional

    // Depth and Stencil testing
    LOG("Depth and Stencil testing, disabled for now, will pass on nullptr");
    // LOGCALL(VkPipelineDepthStencilStateCreateInfo depthStencil{});
    // depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    // depthStencil.depthTestEnable = VK_FALSE;
    // depthStencil.depthWriteEnable = VK_FALSE;

    // Color Blending
    LOG("Color Blending, finalColor = newColor * newAlpha <colorBlendOp> oldColor * (1 - newAlpha)");
    LOG("It is possible to have multiple color blending attachments, have logical ops, and have separate blending for "
        "each color channel.");
    LOGCALL(VkPipelineColorBlendAttachmentState colorBlendAttachment{});
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

    LOGCALL(VkPipelineColorBlendStateCreateInfo colorBlending{});
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    LOG("Setup dynamic states");
    // std::vector<VkDynamicState> dynamicStates = {
    //     VK_DYNAMIC_STATE_VIEWPORT,           VK_DYNAMIC_STATE_LINE_WIDTH,       VK_DYNAMIC_STATE_DEPTH_BIAS,
    //     VK_DYNAMIC_STATE_BLEND_CONSTANTS,    VK_DYNAMIC_STATE_DEPTH_BOUNDS, VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
    //     VK_DYNAMIC_STATE_STENCIL_WRITE_MASK, VK_DYNAMIC_STATE_STENCIL_REFERENCE};

    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Pipeline layout
    LOG("Pipeline Layout, for uniforms and push constants");
    LOGCALL(VkPipelineLayoutCreateInfo pipelineLayoutInfo{});
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;     // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr;  // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (LOGCALL(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout)) != VK_SUCCESS) {
      throw std::runtime_error("failed to create pipeline layout!");
    }

    LOG("Create Graphics Pipeline");
    LOG("Graphics Pipeline, the final pipeline object that will be used in rendering");
    LOG("Here we are combining : shaders, fixed function stages(vertex info, input assembly, viewport syate, "
        "rasterizer, multisampleing, depthStencil and color blending), pipeline layout and render pass");
    LOGCALL(VkGraphicsPipelineCreateInfo pipelineInfo{});
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    // pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
    pipelineInfo.basePipelineIndex = -1;               // Optional

    if (LOGCALL(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline)) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create graphics pipeline!");
    }

    // cleanup
    LOGCALL(vkDestroyShaderModule(device, vertShaderModule, nullptr));
    LOGCALL(vkDestroyShaderModule(device, fragShaderModule, nullptr));
  }

  VkShaderModule createShaderModule(const std::vector<char>& code) {
    LOGFN;
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (LOGCALL(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule)) != VK_SUCCESS) {
      throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
  }

#pragma endregion PIPELINE

#pragma region RENDER_PASS
  void createRenderPass() {
    LOGFN;

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    LOG("clear the values to a constant at the start");
    LOGCALL(colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR);
    LOG("store the values to memory for reading later");
    LOGCALL(colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE);
    LOG("not using stencil buffer");
    LOGCALL(colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    LOGCALL(colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE);
    LOG("layout transition before and after render pass");
    LOGCALL(colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED);
    LOGCALL(colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // Subpasses
    VkAttachmentReference colorAttachmentRef{};
    LOG("which attachment to reference by its index in the attachment descriptions array");
    LOGCALL(colorAttachmentRef.attachment = 0);
    LOG("layout the attachment will have during a subpass");
    LOGCALL(colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VkSubpassDescription subpass{};
    LOG("subpass dependencies, for layout transitions");
    LOGCALL(subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
    LOGCALL(subpass.colorAttachmentCount = 1);
    LOGCALL(subpass.pColorAttachments = &colorAttachmentRef);

    VkSubpassDependency dependency{};
    LOG("dependency between the subpass and the external render pass");
    LOGCALL(dependency.srcSubpass = VK_SUBPASS_EXTERNAL);
    LOGCALL(dependency.dstSubpass = 0);
    LOGCALL(dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    LOGCALL(dependency.srcAccessMask = 0);
    LOGCALL(dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    LOGCALL(dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

    // Render pass
    LOG("Render Pass");
    LOGCALL(VkRenderPassCreateInfo renderPassInfo{});
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (LOGCALL(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass)) != VK_SUCCESS) {
      throw std::runtime_error("failed to create render pass!");
    }
  }

#pragma endregion RENDER_PASS

#pragma region COMMAND_BUFFERS
  void createCommandPool() {
    LOGFN;
    auto queueFamilyIndices = vulkanContext->getQueueFamilyIndices();
    LOGCALL(VkCommandPoolCreateInfo poolInfo{});
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    LOG("Choose graphics family as we are using the command buffer for rendering");
    LOGCALL(poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value());

    if (LOGCALL(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool)) != VK_SUCCESS) {
      throw std::runtime_error("failed to create command pool!");
    }
  }

  void createCommandBuffer() {
    LOGFN;

    LOGCALL(VkCommandBufferAllocateInfo allocInfo{});
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (LOGCALL(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer)) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate command buffers!");
    }
  }

  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    LOGFN_ONCE;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                   // Optional
    beginInfo.pInheritanceInfo = nullptr;  // Optional

    if (LOGCALL_ONCE(vkBeginCommandBuffer(commandBuffer, &beginInfo)) != VK_SUCCESS) {
      throw std::runtime_error("failed to begin recording command buffer!");
    }

    LOG_ONCE("Start Render Pass");
    LOGCALL_ONCE(VkRenderPassBeginInfo renderPassInfo{});
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChain->getFramebuffer(imageIndex);

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    LOGCALL_ONCE(vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE));

    LOG_ONCE("Bind Pipeline");
    LOGCALL_ONCE(vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline));

    LOG_ONCE("Set dynamic states");
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    LOGCALL_ONCE(vkCmdSetViewport(commandBuffer, 0, 1, &viewport));

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    LOGCALL_ONCE(vkCmdSetScissor(commandBuffer, 0, 1, &scissor));

    LOG_ONCE("FINALLY DRAW!!!");
    LOGCALL_ONCE(vkCmdDraw(commandBuffer, 3, 1, 0, 0));

    LOG_ONCE("End Render Pass");
    LOGCALL_ONCE(vkCmdEndRenderPass(commandBuffer));

    if (LOGCALL_ONCE(vkEndCommandBuffer(commandBuffer)) != VK_SUCCESS) {
      throw std::runtime_error("failed to record command buffer!");
    }

    LOG_ONCE("Command Buffer Recorded");
  }

#pragma endregion COMMAND_BUFFERS

#pragma region SYNCHRONISATION
  void createSyncObjects() {
    LOGFN;
    LOG("Synchronization:");
    LOG("Semaphors: signal and wait for the image available and render finished, sync between queues");
    LOG("Fences: wait for the frame to finish before starting the next one, sync between CPU and GPU");

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    LOG("Create fence in signaled state, so that the first frame can start immediately");
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    LOG("vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore)");
    LOG("vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore)");
    LOG("vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence)");
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
      throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
  }
#pragma endregion SYNCHRONISATION

#pragma region DRAW_FRAMES
  void drawFrame() {
    LOGFN_ONCE;
    LOG_ONCE("--------------------------------------------------------------");
    LOG_ONCE("Outline of a frame..");
    LOG_ONCE("Wait for the previous frame to be finished");
    LOG_ONCE("Acquire an image from the swap chain");
    LOG_ONCE("Record a command buffer which draws the scene onto the image.");
    LOG_ONCE("Submit the command buffer to the graphics queue.");
    LOG_ONCE("Present the image to the swap chain for presentation.");
    LOG_ONCE("--------------------------------------------------------------");

    LOG_ONCE("Wait for the previous frame to be finished");
    LOGCALL_ONCE(vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX));
    LOGCALL_ONCE(vkResetFences(device, 1, &inFlightFence));

    LOG_ONCE("Acquire an image from the swap chain");
    uint32_t imageIndex;
    LOGCALL_ONCE(vkAcquireNextImageKHR(device, swapChain->getSwapChain(), UINT64_MAX, imageAvailableSemaphore,
                                       VK_NULL_HANDLE, &imageIndex));

    LOGCALL_ONCE(vkResetCommandBuffer(commandBuffer, 0));
    LOG_ONCE("Record a command buffer which draws the scene onto the image.");
    recordCommandBuffer(commandBuffer, imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    LOG_ONCE("Wait for the imageAvailableSemaphore..");
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    LOG_ONCE("Wait till the color attachment is ready for writing..");
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    LOG_ONCE("Submit the command buffer to the graphics queue");
    if (LOGCALL_ONCE(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence)) != VK_SUCCESS) {
      throw std::runtime_error("failed to submit draw command buffer!");
    }

    // Presentation
    LOG_ONCE("Presentation");
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    LOG_ONCE("Wait for the renderFinishedSemaphore..");
    presentInfo.pWaitSemaphores = signalSemaphores;

    LOG_ONCE("Specify swap chain to present to.");
    VkSwapchainKHR swapChains[] = {swapChain->getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;  // Optional

    LOG_ONCE("Present the image to the swap chain for presentation.");
    if (LOGCALL_ONCE(vkQueuePresentKHR(presentQueue, &presentInfo)) != VK_SUCCESS) {
      throw std::runtime_error("failed to present swap chain image!");
    }

    // firstFrame = false;
  }
#pragma endregion DRAW_FRAMES
};

#pragma region MAIN
int main() {
  LOG("Illiterate Vulkan!");
  App app;
  try {
    app.run();
  } catch (const std::exception& e) {
    LOG("[ERROR]", __FUNCTION__, e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
#pragma endregion MAIN