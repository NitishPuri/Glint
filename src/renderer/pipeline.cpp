#include "pipeline.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

#include "descriptor.h"
#include "logger.h"
#include "mesh.h"
#include "render_pass.h"
#include "swapchain.h"
#include "vk_context.h"

namespace glint {

Pipeline::Pipeline(VkContext* context, RenderPass* renderPass, const std::string& vertShaderPath,
                   const std::string& fragShaderPath, DescriptorSetLayout* descriptorSetLayout)
    : m_Context(context),
      m_RenderPass(renderPass),
      m_PipelineLayout(VK_NULL_HANDLE),
      m_Pipeline(VK_NULL_HANDLE),
      m_DescriptorSetLayout(descriptorSetLayout) {
  LOGFN;
  createGraphicsPipeline(vertShaderPath, fragShaderPath);
}

Pipeline::~Pipeline() {
  LOGFN;
  VkDevice device = m_Context->getDevice();
  LOGCALL(vkDestroyPipeline(device, m_Pipeline, nullptr));
  LOGCALL(vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr));
}

void Pipeline::bind(VkCommandBuffer commandBuffer) {
  LOGFN_ONCE;
  LOG_ONCE("Bind Pipeline");
  LOGCALL_ONCE(vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline));
}

void Pipeline::createGraphicsPipeline(const std::string& vertShaderPath, const std::string& fragShaderPath) {
  LOGFN;

  LOG("Loading shaders, can either load pre-compiled shaders, or compile at runtime to SPIR-V");
  auto vertShaderCode = readFile(vertShaderPath);
  auto fragShaderCode = readFile(fragShaderPath);

  LOGCALL(VkShaderModule vertShaderModule = createShaderModule(vertShaderCode));
  LOGCALL(VkShaderModule fragShaderModule = createShaderModule(fragShaderCode));

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";  // entry point, which means you can control the entry point in the shader
  // vertShaderStageInfo.pSpecializationInfo = nullptr;

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";  // entry point, which means you can control the entry point in the shader
  // fragShaderStageInfo.pSpecializationInfo = nullptr;

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

  LOG("Vertex Input");
  LOGCALL(VkPipelineVertexInputStateCreateInfo vertexInputInfo{});
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  auto bindingDescription = Vertex::getBindingDescription();
  auto attributeDescriptions = Vertex::getAttributeDescriptions();

  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

  LOG("Input Assembly");
  LOGCALL(VkPipelineInputAssemblyStateCreateInfo inputAssembly{});
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;  //
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  LOG("Dynamic States - used for viewport and scissor");
  std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicState.pDynamicStates = dynamicStates.data();

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
  // rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;  // VK_CULL_MODE_FRONT_BIT, VK_CULL_MODE_FRONT_AND_BACK
  rasterizer.cullMode = VK_CULL_MODE_NONE;  // VK_CULL_MODE_FRONT_BIT, VK_CULL_MODE_FRONT_AND_BACK

  // Use counter-clockwise winding order for ALL pipelines
  // This works with both transformed and untransformed geometry with proper setup
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  // rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  LOG("Using counter-clockwise winding for all geometry");

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

  LOG("Global color blending settings");
  LOGCALL(VkPipelineColorBlendStateCreateInfo colorBlending{});
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;  // Optional
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;  // Optional
  colorBlending.blendConstants[1] = 0.0f;  // Optional
  colorBlending.blendConstants[2] = 0.0f;  // Optional
  colorBlending.blendConstants[3] = 0.0f;  // Optional

  LOG("Pipeline Layout");
  LOG("uniform values, push constants, etc.");
  LOGCALL(VkPipelineLayoutCreateInfo pipelineLayoutInfo{});

  if (m_DescriptorSetLayout) {
    LOG("Including descriptor set layout in pipeline layout");

    VkDescriptorSetLayout layout = m_DescriptorSetLayout->getLayout();
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &layout;
  } else {
    LOG("No descriptor set layout for pipeline layout");

    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
  }

  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.pushConstantRangeCount = 0;     // Optional
  pipelineLayoutInfo.pPushConstantRanges = nullptr;  // Optional

  if (LOGCALL(vkCreatePipelineLayout(m_Context->getDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout)) !=
      VK_SUCCESS) {
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
  pipelineInfo.pDepthStencilState = nullptr;  // Optional
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = m_PipelineLayout;
  pipelineInfo.renderPass = m_RenderPass->getRenderPass();
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
  pipelineInfo.basePipelineIndex = -1;               // Optional

  if (LOGCALL(vkCreateGraphicsPipelines(m_Context->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                        &m_Pipeline)) != VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics pipeline!");
  }

  LOG("Cleanup shader modules");
  LOGCALL(vkDestroyShaderModule(m_Context->getDevice(), fragShaderModule, nullptr));
  LOGCALL(vkDestroyShaderModule(m_Context->getDevice(), vertShaderModule, nullptr));
}

VkShaderModule Pipeline::createShaderModule(const std::vector<char>& code) {
  LOGFN;
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(m_Context->getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }

  return shaderModule;
}

std::vector<char> Pipeline::readFile(const std::string& filename) {
  LOGFN;
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("failed to open file: " + filename);
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  LOG("Loading filename:", filename, "fileSize:", fileSize, "bytes");

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();
  return buffer;
}

}  // namespace glint