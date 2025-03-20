#include "render_pass.h"

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

  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = m_SwapChain->getImageFormat();
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

  if (LOGCALL(vkCreateRenderPass(m_Context->getDevice(), &renderPassInfo, nullptr, &m_RenderPass)) != VK_SUCCESS) {
    throw std::runtime_error("failed to create render pass!");
  }
}

void RenderPass::begin(VkCommandBuffer commandBuffer, uint32_t imageIndex, const VkClearValue& clearValue) {
  LOGFN_ONCE;

  LOG_ONCE("Start Render Pass");
  LOGCALL_ONCE(VkRenderPassBeginInfo renderPassInfo{});
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = m_RenderPass;
  renderPassInfo.framebuffer = m_SwapChain->getFramebuffer(imageIndex);

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = m_SwapChain->getExtent();

  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearValue;

  LOGCALL_ONCE(vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE));
}

void RenderPass::end(VkCommandBuffer commandBuffer) {
  LOGFN_ONCE;
  LOG_ONCE("End Render Pass");
  LOGCALL_ONCE(vkCmdEndRenderPass(commandBuffer));
}

}  // namespace glint