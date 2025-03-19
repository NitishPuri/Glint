
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

#include "core/window.h"
#include "logger.h"
#include "vulkan_context.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

std::unordered_set<std::string> glint::OneTimeLogger::loggedFunctions;

static std::vector<char> readFile(const std::string& filename) {
  LOGFN;
  std::ifstream file{filename, std::ios::ate | std::ios::binary};

  if (!file.is_open()) {
    throw std::runtime_error("failed to open file!");
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  LOG("Loading filename:", filename, "fileSize:", fileSize, "bytes");

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();
  return buffer;
}

#pragma region VALIDATION_CALLBACK

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"  //
};

const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* pUserData) {
  if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    // Message is important enough to show
    LOG("[VALIDATION ERROR] validation layer: ", pCallbackData->pMessage);
  } else {
    // Message is not important enough to show
    // LOG("[VALIDATION INFO] validation layer: ", pCallbackData->pMessage);
  }

  return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger) {
  LOGFN;
  LOGCALL(auto func =
              (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
  if (func != nullptr) {
    LOGCALL(return func(instance, pCreateInfo, pAllocator, pDebugMessenger));
  } else {
    LOGCALL(return VK_ERROR_EXTENSION_NOT_PRESENT);
  }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator) {
  LOGFN;
  LOGCALL(auto func =
              (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
  if (func != nullptr) {
    LOGCALL(func(instance, debugMessenger, pAllocator));
  }
}
#pragma endregion VALIDATION_CALLBACK

class App {
 public:
#pragma region APP
  void run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
  }

 private:
  void initWindow() {
    LOGFN;
    LOGCALL(glfwInit());

    LOGCALL(glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API));
    LOGCALL(glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE));

    // LOGCALL(window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr));
    glint::Window::WindowProps props;
    props.title = "Vulkan";
    props.width = WIDTH;
    props.height = HEIGHT;
    props.resizable = false;

    window = std::make_unique<glint::Window>(props);
  }

  void initVulkan() {
    LOGFN;
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createSwapChainBuffers();
    createCommandPool();
    createCommandBuffer();
    createSyncObjects();
  }

  void mainLoop() {
    LOGFN;

    static int frames = 0;
    // LOGCALL(while (!glfwWindowShouldClose(window))) {
    LOGCALL(while (!window->shouldClose())) {
      glfwPollEvents();
      drawFrame();

      frames++;
      // if (frames == 2) break;
    }

    LOGCALL(vkDeviceWaitIdle(device));
  }

  void cleanup() {
    LOGFN;

    LOGCALL(vkDestroySemaphore(device, renderFinishedSemaphore, nullptr));
    LOGCALL(vkDestroySemaphore(device, imageAvailableSemaphore, nullptr));
    LOGCALL(vkDestroyFence(device, inFlightFence, nullptr));

    LOGCALL(vkDestroyCommandPool(device, commandPool, nullptr));
    for (auto framebuffer : swapChainFrameBuffers) {
      LOGCALL(vkDestroyFramebuffer(device, framebuffer, nullptr));
    }

    LOGCALL(vkDestroyPipeline(device, graphicsPipeline, nullptr));
    LOGCALL(vkDestroyPipelineLayout(device, pipelineLayout, nullptr));
    LOGCALL(vkDestroyRenderPass(device, renderPass, nullptr));

    for (auto imageView : swapChainImageViews) {
      LOGCALL(vkDestroyImageView(device, imageView, nullptr));
    }

    LOGCALL(vkDestroySwapchainKHR(device, swapChain, nullptr));
    LOGCALL(vkDestroyDevice(device, nullptr));

    if (enableValidationLayers) {
      DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    LOGCALL(vkDestroySurfaceKHR(instance, surface, nullptr));

    LOGCALL(vkDestroyInstance(instance, nullptr));

    // LOGCALL(glfwDestroyWindow(window));
    // LOGCALL(glfwTerminate());
    // delete window;
    window.reset();
  }

#pragma endregion APP

#pragma region VARIABLES
  std::unique_ptr<glint::Window> window = nullptr;

  VkInstance instance{};
  VkDebugUtilsMessengerEXT debugMessenger;

  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

  VkDevice device;
  VkQueue graphicsQueue;

  VkSurfaceKHR surface;
  VkQueue presentQueue;

  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;

  std::vector<VkImageView> swapChainImageViews;

  VkRenderPass renderPass;
  VkPipelineLayout pipelineLayout;

  VkPipeline graphicsPipeline;

  std::vector<VkFramebuffer> swapChainFrameBuffers;

  VkCommandPool commandPool;
  VkCommandBuffer commandBuffer;

  VkSemaphore imageAvailableSemaphore;
  VkSemaphore renderFinishedSemaphore;
  VkFence inFlightFence;

#pragma endregion VARIABLES

#pragma region INSTANCE
  void createInstance() {
    LOGFN;
    if (enableValidationLayers && !checkValidationLayerSupport()) {
      throw std::runtime_error("validation layers requested, but not available!");
    }

    // Optional
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    //
    LOGCALL(VkInstanceCreateInfo createInfo{});
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
      LOG("validation layers are enabled");
      createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();

      populateDebugMessengerCreateInfo(debugCreateInfo);
      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;  // callback to
                                                                                 // debug
                                                                                 // instance
                                                                                 // creation
    } else {
      createInfo.enabledLayerCount = 0;
      createInfo.pNext = nullptr;
    }

    if (LOGCALL(vkCreateInstance(&createInfo, nullptr, &instance)) != VK_SUCCESS) {
      throw std::runtime_error("failed to create instance!");
    }
  }

  std::vector<const char*> getRequiredExtensions() {
    LOGFN;
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    LOGCALL(glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount));

    if (true) {
      // query all extensions
      uint32_t extensionCount = 0;
      vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
      std::vector<VkExtensionProperties> extensions(extensionCount);
      LOGCALL(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()));
      LOG("Available Extensions: ", extensionCount);

      for (const auto& extension : extensions) {
        LOG("  ", extension.extensionName);
      }
    }

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    LOG("Required Extensions: ", extensions.size());
    for (const auto& extension : extensions) {
      LOG("  ", extension);
    }

    return extensions;
  }
#pragma endregion INSTANCE

#pragma region VALIDATION

  void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
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

  void setupDebugMessenger() {
    LOGFN;
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
      throw std::runtime_error("failed to set up debug messenger!");
    }
  }

  bool checkValidationLayerSupport() {
    LOGFN;
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    LOGCALL(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()));

    for (const char* layerName : validationLayers) {
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

#pragma endregion VALIDATION

#pragma region PHYSICAL_DEVICE

  bool isDeviceSuitable(VkPhysicalDevice device) {
    LOGFN;
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
      SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
      swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
  }

  bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    LOGFN;
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    LOGCALL(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()));

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
      requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
  }

  void pickPhysicalDevice() {
    LOGFN;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
      throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    LOGCALL(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));

    LOG("Available Devices: ", deviceCount);

    for (const auto& device : devices) {
      if (isDeviceSuitable(device)) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        LOG("found suitable device ", deviceProperties.deviceName);

        physicalDevice = device;

        break;
      }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
      throw std::runtime_error("failed to find a suitable GPU!");
    }
  }

  struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
  };
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    LOGFN;
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    LOG("queueFamilyCount :", queueFamilyCount);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    LOGCALL(vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data()));

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
      VkBool32 presentSupport = false;
      LOGCALL(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport));
      if (presentSupport) {
        indices.presentFamily = i;
      }

      if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        indices.graphicsFamily = i;
      }

      if (indices.isComplete()) {
        break;
      }
    }

    LOG("found queue families", indices.graphicsFamily.value(), indices.presentFamily.value());
    return indices;
  }

#pragma endregion PHYSICAL_DEVICE

#pragma region LOGICAL_DEVICE
  void createLogicalDevice() {
    LOGFN;
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;  // yo dawg, we only need one queue

      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (LOGCALL(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device)) != VK_SUCCESS) {
      throw std::runtime_error("failed to create logical device!");
    }

    // get device queue handle
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
  }
#pragma endregion LOGICAL_DEVICE

#pragma region SURFACE
  void createSurface() {
    // LOGFN;
    // if (LOGCALL(glfwCreateWindowSurface(instance, window, nullptr, &surface)) != VK_SUCCESS) {
    //   throw std::runtime_error("failed to create window surface!");
    // }
    surface = window->createSurface(instance);
  }
#pragma endregion SURFACE

#pragma region SWAPCHAIN
  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
    LOGFN;
    SwapChainSupportDetails details;

    LOGCALL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities));

    uint32_t formatCount;
    LOGCALL(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr));
    if (formatCount != 0) {
      details.formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    LOGCALL(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr));
    if (presentModeCount != 0) {
      details.presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
  }

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    LOGFN;
    for (const auto& availableFormat : availableFormats) {
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
          availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        return availableFormat;
      }
    }

    return availableFormats[0];
  }

  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    LOGFN;
    for (const auto& availablePresentMode : availablePresentModes) {
      // Use VK_PRESENT_MODE_MAILBOX_KHR for triple buffering id enerygy is not a concern
      // Use VK_PRESENT_MODE_FIFO_KHR for double buffering, preferable on mobile devices
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
        return availablePresentMode;
      }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
  }

  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    LOGFN;
    if (capabilities.currentExtent.width != UINT32_MAX) {
      return capabilities.currentExtent;
    } else {
      // int width, height;
      // LOGCALL(glfwGetFramebufferSize(window, &width, &height));
      uint32_t width = window->getWidth();
      uint32_t height = window->getHeight();

      // LOG("window size: ", WIDTH, "x", HEIGHT);
      LOG("frame buffer size ", width, "x", height);

      VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

      actualExtent.width =
          std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
      actualExtent.height =
          std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

      return actualExtent;
    }
  }

  void createSwapChain() {
    LOGFN;
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    // recommended to request at least one more image than the minimum
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
      imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;  // always 1, unless Stereo 3D!!
    createInfo.imageUsage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // means that we render directly to images in this swapchain
    LOG("imageUsage: VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT");

    // for rendering to a separate image first,
    // then copy to the swapchain image, use VK_IMAGE_USAGE_TRANSFER_DST_BIT
    // for post-processing, use VK_IMAGE_USAGE_TRANSFER_SRC_BIT

    // for sharing images between queues
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    if (indices.graphicsFamily != indices.presentFamily) {
      LOG("graphics and present queues are different");
      LOGCALL(createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT);
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
      LOG("graphics and present queues are the same");
      LOGCALL(createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE);
      createInfo.queueFamilyIndexCount = 0;      // Optional
      createInfo.pQueueFamilyIndices = nullptr;  // Optional
    }

    // Used for transformations (eg. 90 degree rotation for mobile devices)
    LOGCALL(createInfo.preTransform = swapChainSupport.capabilities.currentTransform);
    // QUESTION: Can support transparent windows using this flag??
    LOGCALL(createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);
    LOGCALL(createInfo.presentMode = presentMode);
    // This is window clipping, i.e. if some other window is in front of this one
    // then we can clip the rendering to the visible area
    createInfo.clipped = VK_TRUE;

    // QUESTION: What is the purpose of oldSwapChain?
    LOGCALL(createInfo.oldSwapchain = VK_NULL_HANDLE);

    if (LOGCALL(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain)) != VK_SUCCESS) {
      throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    LOGCALL(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data()));
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
  }

#pragma endregion SWAPCHAIN

#pragma region IMAGE_VIEW
  void createImageViews() {
    LOGFN;
    LOGCALL(swapChainImageViews.resize(swapChainImages.size()));
    LOG("swapChainImages.size(): ", swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
      VkImageViewCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      createInfo.image = swapChainImages[i];
      LOGCALL(createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D);
      createInfo.format = swapChainImageFormat;
      LOG("format: ", swapChainImageFormat);

      LOG("components: VK_COMPONENT_SWIZZLE_IDENTITY");
      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

      LOG("No mipmapping or multiple layers");
      LOGCALL(createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);
      createInfo.subresourceRange.baseMipLevel = 0;
      createInfo.subresourceRange.levelCount = 1;
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount = 1;

      if (LOGCALL(vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i])) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image views!");
      }
    }
  }

#pragma endregion IMAGE_VIEW

#pragma region PIPELINE
  void createGraphicsPipeline() {
    LOGFN;

    LOG("Loading shaders, can wither load pre compiled shaders, or compile at runtime to SPIR-V");
    auto vertShaderCode = readFile("./bin/shaders/shader.vert.spv");
    auto fragShaderCode = readFile("./bin/shaders/shader.frag.spv");

    LOGCALL(VkShaderModule vertShaderModule = createShaderModule(vertShaderCode));
    LOGCALL(VkShaderModule fragShaderModule = createShaderModule(fragShaderCode));

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";  // entry point, whihch means you can control the entry point in the shader
    // vertShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";  // entry point, whihch means you can control the entry point in the shader
    // fragShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    LOG("Vertex Input");
    LOGCALL(VkPipelineVertexInputStateCreateInfo vertexInputInfo{});
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;  // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;  // Optional

    LOG("Input Assembly");
    LOGCALL(VkPipelineInputAssemblyStateCreateInfo inputAssembly{});
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;  //
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewports and scissors - if dynamic states are not used
    // LOGCALL(VkViewport viewport{});
    // viewport.x = 0.0f;
    // viewport.y = 0.0f;
    // viewport.width = (float)swapChainExtent.width;
    // viewport.height = (float)swapChainExtent.height;
    // viewport.minDepth = 0.0f;
    // viewport.maxDepth = 1.0f;

    // VkRect2D scissor{};
    // scissor.offset = {0, 0};
    // scissor.extent = swapChainExtent;

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
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;     // VK_CULL_MODE_FRONT_BIT, VK_CULL_MODE_FRONT_AND_BACK
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;  // VK_FRONT_FACE_COUNTER_CLOCKWISE
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

    LOGCALL(VkPipelineColorBlendStateCreateInfo colorBlending{});
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    LOG("Setup dynamic states");
    // std::vector<VkDynamicState> dynamicStates = {
    //     VK_DYNAMIC_STATE_VIEWPORT,           VK_DYNAMIC_STATE_LINE_WIDTH,       VK_DYNAMIC_STATE_DEPTH_BIAS,
    //     VK_DYNAMIC_STATE_BLEND_CONSTANTS,    VK_DYNAMIC_STATE_DEPTH_BOUNDS, VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
    //     VK_DYNAMIC_STATE_STENCIL_WRITE_MASK, VK_DYNAMIC_STATE_STENCIL_REFERENCE};

    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Pipeline layout
    LOG("Pipeline Layout, for uniforms and push constants");
    LOGCALL(VkPipelineLayoutCreateInfo pipelineLayoutInfo{});
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;     // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr;  // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (LOGCALL(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout)) != VK_SUCCESS) {
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
    // pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
    pipelineInfo.basePipelineIndex = -1;               // Optional

    if (LOGCALL(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline)) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create graphics pipeline!");
    }

    // cleanup
    LOGCALL(vkDestroyShaderModule(device, vertShaderModule, nullptr));
    LOGCALL(vkDestroyShaderModule(device, fragShaderModule, nullptr));
  }

  VkShaderModule createShaderModule(const std::vector<char>& code) {
    LOGFN;
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (LOGCALL(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule)) != VK_SUCCESS) {
      throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
  }

#pragma endregion PIPELINE

#pragma region RENDER_PASS
  void createRenderPass() {
    LOGFN;

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
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

    if (LOGCALL(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass)) != VK_SUCCESS) {
      throw std::runtime_error("failed to create render pass!");
    }
  }

#pragma endregion RENDER_PASS

#pragma region FRAME_BUFFERS
  void createSwapChainBuffers() {
    LOGFN;
    LOGCALL(swapChainFrameBuffers.resize(swapChainImageViews.size()));

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
      VkImageView attachments[] = {swapChainImageViews[i]};

      VkFramebufferCreateInfo framebufferInfo{};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = renderPass;
      framebufferInfo.attachmentCount = 1;
      framebufferInfo.pAttachments = attachments;
      framebufferInfo.width = swapChainExtent.width;
      framebufferInfo.height = swapChainExtent.height;
      framebufferInfo.layers = 1;

      if (LOGCALL(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFrameBuffers[i])) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
      }
    }
  }
#pragma endregion FRAME_BUFFERS

#pragma region COMMAND_BUFFERS
  void createCommandPool() {
    LOGFN;
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    LOGCALL(VkCommandPoolCreateInfo poolInfo{});
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    LOG("Choose graphics family as we are using the command buffer for rendering");
    LOGCALL(poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value());

    if (LOGCALL(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool)) != VK_SUCCESS) {
      throw std::runtime_error("failed to create command pool!");
    }
  }

  void createCommandBuffer() {
    LOGFN;

    LOGCALL(VkCommandBufferAllocateInfo allocInfo{});
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (LOGCALL(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer)) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate command buffers!");
    }
  }

  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    LOGFN_ONCE;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                   // Optional
    beginInfo.pInheritanceInfo = nullptr;  // Optional

    if (LOGCALL_ONCE(vkBeginCommandBuffer(commandBuffer, &beginInfo)) != VK_SUCCESS) {
      throw std::runtime_error("failed to begin recording command buffer!");
    }

    LOG_ONCE("Start Render Pass");
    LOGCALL_ONCE(VkRenderPassBeginInfo renderPassInfo{});
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFrameBuffers[imageIndex];

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    LOGCALL_ONCE(vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE));

    LOG_ONCE("Bind Pipeline");
    LOGCALL_ONCE(vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline));

    LOG_ONCE("Set dynamic states");
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    LOGCALL_ONCE(vkCmdSetViewport(commandBuffer, 0, 1, &viewport));

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    LOGCALL_ONCE(vkCmdSetScissor(commandBuffer, 0, 1, &scissor));

    LOG_ONCE("FINALLY DRAW!!!");
    LOGCALL_ONCE(vkCmdDraw(commandBuffer, 3, 1, 0, 0));

    LOG_ONCE("End Render Pass");
    LOGCALL_ONCE(vkCmdEndRenderPass(commandBuffer));

    if (LOGCALL_ONCE(vkEndCommandBuffer(commandBuffer)) != VK_SUCCESS) {
      throw std::runtime_error("failed to record command buffer!");
    }

    LOG_ONCE("Command Buffer Recorded");
  }

#pragma endregion COMMAND_BUFFERS

#pragma region SYNCHRONISATION
  void createSyncObjects() {
    LOGFN;
    LOG("Synchronization:");
    LOG("Semaphors: signal and wait for the image available and render finished, sync between queues");
    LOG("Fences: wait for the frame to finish before starting the next one, sync between CPU and GPU");

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    LOG("Create fence in signaled state, so that the first frame can start immediately");
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    LOG("vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore)");
    LOG("vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore)");
    LOG("vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence)");
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
      throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
  }
#pragma endregion SYNCHRONISATION

#pragma region DRAW_FRAMES
  void drawFrame() {
    LOGFN_ONCE;
    LOG_ONCE("--------------------------------------------------------------");
    LOG_ONCE("Outline of a frame..");
    LOG_ONCE("Wait for the previous frame to be finished");
    LOG_ONCE("Acquire an image from the swap chain");
    LOG_ONCE("Record a command buffer which draws the scene onto the image.");
    LOG_ONCE("Submit the command buffer to the graphics queue.");
    LOG_ONCE("Present the image to the swap chain for presentation.");
    LOG_ONCE("--------------------------------------------------------------");

    LOG_ONCE("Wait for the previous frame to be finished");
    LOGCALL_ONCE(vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX));
    LOGCALL_ONCE(vkResetFences(device, 1, &inFlightFence));

    LOG_ONCE("Acquire an image from the swap chain");
    uint32_t imageIndex;
    LOGCALL_ONCE(
        vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex));

    LOGCALL_ONCE(vkResetCommandBuffer(commandBuffer, 0));
    LOG_ONCE("Record a command buffer which draws the scene onto the image.");
    recordCommandBuffer(commandBuffer, imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    LOG_ONCE("Wait for the imageAvailableSemaphore..");
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    LOG_ONCE("Wait till the color attachment is ready for writing..");
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    LOG_ONCE("Submit the command buffer to the graphics queue");
    if (LOGCALL_ONCE(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence)) != VK_SUCCESS) {
      throw std::runtime_error("failed to submit draw command buffer!");
    }

    // Presentation
    LOG_ONCE("Presentation");
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    LOG_ONCE("Wait for the renderFinishedSemaphore..");
    presentInfo.pWaitSemaphores = signalSemaphores;

    LOG_ONCE("Specify swap chain to present to.");
    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;  // Optional

    LOG_ONCE("Present the image to the swap chain for presentation.");
    if (LOGCALL_ONCE(vkQueuePresentKHR(presentQueue, &presentInfo)) != VK_SUCCESS) {
      throw std::runtime_error("failed to present swap chain image!");
    }

    // firstFrame = false;
  }
#pragma endregion DRAW_FRAMES
};

#pragma region MAIN
int main() {
  LOG("Illiterate Vulkan!");
  App app;
  try {
    app.run();
  } catch (const std::exception& e) {
    LOG("[ERROR]", __FUNCTION__, e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
#pragma endregion MAIN