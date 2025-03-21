#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

#include "vertex.h"

namespace glint {

class VkContext;
class SwapChain;
class RenderPass;

class DescriptorSetLayout;

struct PipelineConfig {
  // Shaders
  std::string vertexShaderPath;  // add default shaders
  std::string fragmentShaderPath;

  // Vertex input
  VertexAttributeFlags vertexFormat = VertexAttributeFlags::POSITION_COLOR_TEXCOORD;

  // Descriptors
  DescriptorSetLayout* descriptorSetLayout = nullptr;

  // Topology
  VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  // Depth settings
  // bool depthTestEnable = false;
  // bool depthWriteEnable = false;
  // VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;

  // Rasterization settings
  VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;

  // Use counter-clockwise winding order for ALL pipelines
  // This works with both transformed and untransformed geometry with proper setup
  VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

  // VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
  // float lineWidth = 1.0f;

  // Blending settings
  // bool blendEnable = false;
  // VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  // VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  // VkBlendOp colorBlendOp = VK_BLEND_OP_ADD;
  // VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  // VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  // VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD;

  // Push constants
  // std::vector<VkPushConstantRange> pushConstantRanges;
};

class Pipeline {
 public:
  Pipeline(VkContext* context, RenderPass* renderPass, const PipelineConfig* config);
  ~Pipeline();

  // Prevent copying
  Pipeline(const Pipeline&) = delete;
  Pipeline& operator=(const Pipeline&) = delete;

  // Getters
  VkPipeline getPipeline() const { return m_Pipeline; }
  VkPipelineLayout getPipelineLayout() const { return m_PipelineLayout; }

  void bind(VkCommandBuffer commandBuffer);

 private:
  void createGraphicsPipeline(const PipelineConfig& config);

  VkShaderModule createShaderModule(const std::vector<char>& code);
  std::vector<char> readFile(const std::string& filename);

 private:
  VkContext* m_Context;
  RenderPass* m_RenderPass;
  VkPipelineLayout m_PipelineLayout;
  VkPipeline m_Pipeline;
};

}  // namespace glint