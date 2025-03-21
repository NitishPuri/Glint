#pragma once

#include <vulkan/vulkan.h>

#include <optional>
#include <string>
#include <vector>

#include "logger.h"

namespace glint {

class Window;

// handles instance, debug messenger, surface, and device creation, and manages the lifecycle of these objects
class VkContext {
 public:
  // VulkanContext(Window* window, const std::string& appName = "Glint Engine");
  VkContext(Window* window);
  ~VkContext();

  // Prevent copying
  VkContext(const VkContext&) = delete;
  VkContext& operator=(const VkContext&) = delete;

  // Getters
  VkInstance getInstance() const { return m_Instance; }
  VkPhysicalDevice getPhysicalDevice() const { return m_PhysicalDevice; }
  VkDevice getDevice() const { return m_Device; }
  VkQueue getGraphicsQueue() const { return m_GraphicsQueue; }
  VkQueue getPresentQueue() const { return m_PresentQueue; }
  VkSurfaceKHR getSurface() const { return m_Surface; }

  Window* getWindow() const { return m_Window; }

  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

  // TODO: Remove this, try to use the command manager insttead whereever this is used.
  VkCommandPool getCommandPool() const { return m_CommandPool; }
  void setCommandPool(VkCommandPool commamndPool) { m_CommandPool = commamndPool; }

  VkSampleCountFlagBits getMsaaSamples() const { return m_MsaaSamples; }

  struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
  };

  QueueFamilyIndices getQueueFamilyIndices() const { return m_QueueFamilyIndices; }

  void init();
  void cleanup();

 private:
  // void createInstance(const std::string& appName);
  void createInstance();

  void setupDebugMessenger();
  void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();

  VkSampleCountFlagBits getMaxUsableSampleCount();

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

  VkSampleCountFlagBits m_MsaaSamples = VK_SAMPLE_COUNT_1_BIT;

  VkCommandPool m_CommandPool;

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