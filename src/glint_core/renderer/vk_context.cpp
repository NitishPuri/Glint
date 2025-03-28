#include "vk_context.h"

#include <set>
#include <stdexcept>

#include "core/config.h"
#include "core/logger.h"
#include "core/window.h"
#include "renderer/vk_utils.h"

namespace glint {

// Static callback function for debug messenger
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* pUserData) {
  if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    // Message is important enough to show
    if (Config::isLoggingEnabled())
      LOG("[VALIDATION ERROR] validation layer: ", pCallbackData->pMessage);
    else
      std::cout << "[VALIDATION ERROR] validation layer: " << pCallbackData->pMessage << std::endl;
  } else {
    // Message is not important enough to show
    // LOG("[VALIDATION INFO] validation layer: ", pCallbackData->pMessage);
  }

  return VK_FALSE;
}

// Helper functions for debug messenger
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger) {
  LOGFN;
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator) {
  LOGFN;
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}

VkContext::VkContext(Window* window) : m_Window(window) { LOGFN; }

VkContext::~VkContext() {
  LOGFN;
  cleanup();
}

void VkContext::init() {
  LOGFN;
  createInstance();
  setupDebugMessenger();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
}

void VkContext::cleanup() {
  LOGFN;
  vkDestroyDevice(m_Device, nullptr);

  if (m_EnableValidationLayers) {
    DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
  }

  vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
  vkDestroyInstance(m_Instance, nullptr);
}

void VkContext::createInstance() {
  LOGFN;
  if (m_EnableValidationLayers && !checkValidationLayerSupport()) {
    throw std::runtime_error("validation layers requested, but not available!");
  }

  // Application info
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Glint Engine";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "Glint";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  // Instance create info
  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  auto extensions = getRequiredExtensions();
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
  if (m_EnableValidationLayers) {
    LOG("validation layers are enabled");
    createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
    createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

    populateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
  } else {
    LOG("no validation layers.");
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }

  if (LOGCALL(vkCreateInstance(&createInfo, nullptr, &m_Instance)) != VK_SUCCESS) {
    throw std::runtime_error("failed to create Vulkan instance!");
  }
}

void VkContext::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
  LOGFN;
  createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
}

void VkContext::setupDebugMessenger() {
  LOGFN;
  if (!m_EnableValidationLayers) return;

  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessengerCreateInfo(createInfo);

  if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug messenger!");
  }
}

void VkContext::createSurface() {
  LOGFN;
  m_Surface = m_Window->createSurface(m_Instance);
}

void VkContext::pickPhysicalDevice() {
  LOGFN;
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

  if (Config::isLoggingEnabled() && Config::isOptionSet("list_gpus")) {
    LOG("Available GPUs:", deviceCount);
    for (const auto& device : devices) {
      VkPhysicalDeviceProperties deviceProperties;
      vkGetPhysicalDeviceProperties(device, &deviceProperties);
      LOG("Device   : ", deviceProperties.deviceName);
      LOG("DeviceID : ", deviceProperties.deviceID);
      LOG("DeviceType : ", glint::tools::physicalDeviceTypeString(deviceProperties.deviceType));
      LOG("API Version : ", (deviceProperties.apiVersion >> 22), ".", ((deviceProperties.apiVersion >> 12) & 0x3ff),
          ".", (deviceProperties.apiVersion & 0xfff));
      LOG("===============================");
    }
  }

  for (const auto& device : devices) {
    if (isDeviceSuitable(device)) {
      VkPhysicalDeviceProperties deviceProperties;
      vkGetPhysicalDeviceProperties(device, &deviceProperties);
      LOG("found suitable device ", deviceProperties.deviceName);

      m_PhysicalDevice = device;
      m_MsaaSamples = getMaxUsableSampleCount();
      LOG("Max Usable Sample Count: ", m_MsaaSamples);
      break;
    }
  }

  if (m_PhysicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
}

void VkContext::createLogicalDevice() {
  LOGFN;
  m_QueueFamilyIndices = findQueueFamilies(m_PhysicalDevice);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {m_QueueFamilyIndices.graphicsFamily.value(),
                                            m_QueueFamilyIndices.presentFamily.value()};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.samplerAnisotropy = VK_TRUE;
  deviceFeatures.sampleRateShading = VK_TRUE;

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size());
  createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

  if (LOGCALL(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device)) != VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }

  // Get device queue handles
  vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.graphicsFamily.value(), 0, &m_GraphicsQueue);
  vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.presentFamily.value(), 0, &m_PresentQueue);
}

bool VkContext::checkValidationLayerSupport() {
  LOGFN;
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char* layerName : m_ValidationLayers) {
    bool layerFound = false;

    for (const auto& layerProperties : availableLayers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        LOG("found validation layer: ", layerName);
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      return false;
    }
  }

  return true;
}

std::vector<const char*> VkContext::getRequiredExtensions() {
  LOGFN;
  std::vector<const char*> extensions = m_Window->getRequiredInstanceExtensions();

  if (m_EnableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  LOG("Required Extensions: ", extensions.size());
  for (const auto& extension : extensions) {
    LOG("  ", extension);
  }

  return extensions;
}

bool VkContext::isDeviceSuitable(VkPhysicalDevice device) {
  LOGFN;
  QueueFamilyIndices indices = findQueueFamilies(device);

  bool extensionsSupported = checkDeviceExtensionSupport(device);

  bool swapChainAdequate = false;
  if (extensionsSupported) {
    // SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

    // swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    swapChainAdequate = formatCount > 0 && presentModeCount > 0;
  }

  VkPhysicalDeviceFeatures supportedFatures;
  vkGetPhysicalDeviceFeatures(device, &supportedFatures);

  return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFatures.samplerAnisotropy;
}

bool VkContext::checkDeviceExtensionSupport(VkPhysicalDevice device) {
  LOGFN;
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

  for (const auto& extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

VkContext::QueueFamilyIndices VkContext::findQueueFamilies(VkPhysicalDevice device) {
  LOGFN;
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
  //   LOG("queueFamilyCount: ", queueFamilyCount);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    // Check for present support
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);
    if (presentSupport) {
      indices.presentFamily = i;
    }

    // Check for graphics support
    if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
    }

    if (indices.isComplete()) {
      break;
    }
  }

  return indices;
}

uint32_t VkContext::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

VkSampleCountFlagBits VkContext::getMaxUsableSampleCount() {
  VkPhysicalDeviceProperties physicalDeviceProperties;
  vkGetPhysicalDeviceProperties(m_PhysicalDevice, &physicalDeviceProperties);

  VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
                              physicalDeviceProperties.limits.framebufferDepthSampleCounts;
  if (counts & VK_SAMPLE_COUNT_64_BIT) {
    return VK_SAMPLE_COUNT_64_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_32_BIT) {
    return VK_SAMPLE_COUNT_32_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_16_BIT) {
    return VK_SAMPLE_COUNT_16_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_8_BIT) {
    return VK_SAMPLE_COUNT_8_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_4_BIT) {
    return VK_SAMPLE_COUNT_4_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_2_BIT) {
    return VK_SAMPLE_COUNT_2_BIT;
  }

  return VK_SAMPLE_COUNT_1_BIT;
}

VkPhysicalDeviceProperties VkContext::getPhysicalDeviceProperties() const {
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(m_PhysicalDevice, &properties);
  return properties;
}

}  // namespace glint