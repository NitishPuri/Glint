#pragma once

#include <vulkan/vulkan.h>

#include <string>

namespace glint {

class VkContext;
class CommandManager;

class Texture {
 public:
  Texture(VkContext* context, const std::string& filepath);
  ~Texture();

  // Prevent copying
  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;

  VkImageView getImageView() const { return m_ImageView; }
  VkSampler getSampler() const { return m_Sampler; }
  bool isValid() const { return m_Image != VK_NULL_HANDLE; }

 private:
  void createTextureImage(const std::string& filepath);
  void createTextureImageView();
  void createTextureSampler();

  void generateMipmaps();

 private:
  VkContext* m_Context;

  uint32_t m_mipLevels = 1;
  uint32_t m_Width = 0;
  uint32_t m_Height = 0;

  // TODO: Wrap Buffer and Buffer Memory together?
  VkImage m_Image = VK_NULL_HANDLE;
  VkDeviceMemory m_ImageMemory = VK_NULL_HANDLE;

  VkImageView m_ImageView = VK_NULL_HANDLE;
  VkSampler m_Sampler = VK_NULL_HANDLE;
};

}  // namespace glint