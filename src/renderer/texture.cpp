#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// #include "buffer.h"
#include "command_manager.h"
#include "logger.h"
#include "vk_context.h"
#include "vk_utils.h"

namespace glint {

Texture::Texture(VkContext* context, const std::string& filepath) : m_Context(context) {
  LOGFN;
  createTextureImage(filepath);
  createTextureImageView();
  createTextureSampler();
}

Texture::~Texture() {
  LOGFN;
  VkDevice device = m_Context->getDevice();

  if (m_Sampler != VK_NULL_HANDLE) {
    vkDestroySampler(device, m_Sampler, nullptr);
    m_Sampler = VK_NULL_HANDLE;
  }

  if (m_ImageView != VK_NULL_HANDLE) {
    vkDestroyImageView(device, m_ImageView, nullptr);
    m_ImageView = VK_NULL_HANDLE;
  }

  if (m_Image != VK_NULL_HANDLE) {
    vkDestroyImage(device, m_Image, nullptr);
    m_Image = VK_NULL_HANDLE;
  }

  if (m_ImageMemory != VK_NULL_HANDLE) {
    vkFreeMemory(device, m_ImageMemory, nullptr);
    m_ImageMemory = VK_NULL_HANDLE;
  }
}

void Texture::createTextureImage(const std::string& filepath) {
  LOGFN;
  LOG("Loading texture from", filepath);

  // Load image with STB
  int texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;  // 4 bytes per RGBA pixel

  if (!pixels) {
    LOG("[ERROR] Failed to load texture image from", filepath);
    throw std::runtime_error("Failed to load texture image!");
  }

  LOG("Texture loaded:", texWidth, "x", texHeight, "pixels,", texChannels, "channels");

  m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  VkUtils::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                        stagingBufferMemory);
  VkUtils::setObjectName((uint64_t)stagingBuffer, VK_OBJECT_TYPE_BUFFER, "Texture Staging Buffer");
  VkUtils::setObjectName((uint64_t)stagingBufferMemory, VK_OBJECT_TYPE_DEVICE_MEMORY, "Texture Staging Buffer Memory");

  // Copy data to staging buffer
  void* data;
  vkMapMemory(m_Context->getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(m_Context->getDevice(), stagingBufferMemory);

  // Free the pixel data
  stbi_image_free(pixels);

  VkUtils::createImage(static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), m_mipLevels,
                       VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                       VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Image, m_ImageMemory);
  VkUtils::setObjectName((uint64_t)m_Image, VK_OBJECT_TYPE_IMAGE, "Texture Buffer");
  VkUtils::setObjectName((uint64_t)m_ImageMemory, VK_OBJECT_TYPE_DEVICE_MEMORY, "Texture Buffer Memory");

  // Transition image layout and copy data
  VkUtils::transitionImageLayout(m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, m_mipLevels);
  VkUtils::copyBufferToImage(stagingBuffer, m_Image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
  VkUtils::transitionImageLayout(m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, m_mipLevels);

  // cleanup
  vkDestroyBuffer(m_Context->getDevice(), stagingBuffer, nullptr);
  vkFreeMemory(m_Context->getDevice(), stagingBufferMemory, nullptr);
}

void Texture::createTextureImageView() {
  m_ImageView = VkUtils::createImageView(m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_mipLevels);
}

void Texture::createTextureSampler() {
  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_TRUE;

  // Query the physical device properties for max anisotropy
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(m_Context->getPhysicalDevice(), &properties);
  samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 0.0f;

  if (vkCreateSampler(m_Context->getDevice(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create texture sampler!");
  }
}

}  // namespace glint