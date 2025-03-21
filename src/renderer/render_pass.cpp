#include "render_pass.h"

#include <array>
#include <stdexcept>

#include "../logger.h"
#include "swapchain.h"
#include "vk_context.h"

namespace glint {

RenderPass::RenderPass(VkContext* context, SwapChain* swapChain)
    : m_Context(context), m_SwapChain(swapChain), m_RenderPass(VK_NULL_HANDLE) {
  LOGFN;
  createRenderPass();
}

RenderPass::~RenderPass() {
  LOGFN;
  LOGCALL(vkDestroyRenderPass(m_Context->getDevice(), m_RenderPass, nullptr));
}

void RenderPass::createRenderPass() {
  LOGFN;

  // Color Attachment
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = m_SwapChain->getImageFormat();
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  // Depth attachment
  VkAttachmentDescription depthAttachment{};
  depthAttachment.format = m_SwapChain->getDepthFormat();
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // Subpasses
  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  // Subpass dependency
  // dependency between the subpass and the external render pass
  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  // Combine attachments
  std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

  // Render pass
  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (LOGCALL(vkCreateRenderPass(m_Context->getDevice(), &renderPassInfo, nullptr, &m_RenderPass)) != VK_SUCCESS) {
    throw std::runtime_error("failed to create render pass!");
  }
}

void RenderPass::begin(VkCommandBuffer commandBuffer, uint32_t imageIndex, const VkClearColorValue& clearColor) {
  LOGFN_ONCE;
  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = clearColor;
  clearValues[1].depthStencil = {1.0f, 0};  // Default depth clear value (far plane)

  LOG_ONCE("Start Render Pass");
  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = m_RenderPass;
  renderPassInfo.framebuffer = m_SwapChain->getFramebuffer(imageIndex);

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = m_SwapChain->getExtent();

  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  LOGCALL_ONCE(vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE));
}

void RenderPass::end(VkCommandBuffer commandBuffer) {
  LOGFN_ONCE;
  LOG_ONCE("End Render Pass");
  LOGCALL_ONCE(vkCmdEndRenderPass(commandBuffer));
}

}  // namespace glint