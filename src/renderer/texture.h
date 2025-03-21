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

 private:
  void createTextureImage(const std::string& filepath);
  void createTextureImageView();
  void createTextureSampler();

 private:
  VkContext* m_Context;

  // TODO: Wrap Buffer and Buffer Memory together?
  VkImage m_Image = VK_NULL_HANDLE;
  VkDeviceMemory m_ImageMemory = VK_NULL_HANDLE;

  VkImageView m_ImageView = VK_NULL_HANDLE;
  VkSampler m_Sampler = VK_NULL_HANDLE;
};

}  // namespace glint