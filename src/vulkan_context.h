#pragma once

#include <vulkan/vulkan.h>

#include <optional>
#include <string>
#include <vector>

namespace glint {

class Window;

class VulkanContext {
 public:
  VulkanContext(Window* window, const std::string& appName = "Glint Engine");
  ~VulkanContext();

  // Prevent copying
  VulkanContext(const VulkanContext&) = delete;
  VulkanContext& operator=(const VulkanContext&) = delete;

  // Getters
  VkInstance getInstance() const { return m_Instance; }
  VkPhysicalDevice getPhysicalDevice() const { return m_PhysicalDevice; }
  VkDevice getDevice() const { return m_Device; }
  VkQueue getGraphicsQueue() const { return m_GraphicsQueue; }
  VkQueue getPresentQueue() const { return m_PresentQueue; }
  VkSurfaceKHR getSurface() const { return m_Surface; }

  struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
  };

  QueueFamilyIndices getQueueFamilyIndices() const { return m_QueueFamilyIndices; }

 private:
  void createInstance(const std::string& appName);
  void setupDebugMessenger();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();

  bool checkValidationLayerSupport();
  std::vector<const char*> getRequiredExtensions();
  bool isDeviceSuitable(VkPhysicalDevice device);
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

 private:
  Window* m_Window;
  VkInstance m_Instance;
  VkDebugUtilsMessengerEXT m_DebugMessenger;
  VkSurfaceKHR m_Surface;
  VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
  VkDevice m_Device;
  VkQueue m_GraphicsQueue;
  VkQueue m_PresentQueue;
  QueueFamilyIndices m_QueueFamilyIndices;

  // Constants
  const std::vector<const char*> m_ValidationLayers = {"VK_LAYER_KHRONOS_validation"};
  const std::vector<const char*> m_DeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
  const bool m_EnableValidationLayers = false;
#else
  const bool m_EnableValidationLayers = true;
#endif
};

}  // namespace glint