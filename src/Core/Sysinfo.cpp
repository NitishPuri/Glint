#include <windows.h>

#include "Window.h"

void printSysinfo() {
  const GLubyte* renderer = glGetString(GL_RENDERER);
  const GLubyte* vendor = glGetString(GL_VENDOR);
  const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
  const GLubyte* version = glGetString(GL_VERSION);

  if (!version || !renderer || !vendor || !glslVersion) {
    Logger::error("Failed to get system info");
    return;
  }

  if (version) Logger::log("Version: ", version);
  if (renderer) Logger::log("Renderer: ", renderer);
  if (vendor) Logger::log("Vendor: ", vendor);
  if (glslVersion) Logger::log("GLSL Version: ", glslVersion);

  int count;
  GLFWmonitor** monitors = glfwGetMonitors(&count);
  Logger::log("Number of monitors: ", count);
  for (int i = 0; i < count; ++i) {
    const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
    Logger::log("Monitor ", i + 1, ": ", mode->width, "x", mode->height, " @ ", mode->refreshRate, "Hz");
  }

  // Print system memory info
  MEMORYSTATUSEX memInfo;
  memInfo.dwLength = sizeof(MEMORYSTATUSEX);
  GlobalMemoryStatusEx(&memInfo);
  Logger::log("Total System Memory: ", memInfo.ullTotalPhys / (1024 * 1024), " MB");
  Logger::log("Available System Memory: ", memInfo.ullAvailPhys / (1024 * 1024), " MB");

  // Print CPU info
  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);
  Logger::log("Number of Processors: ", sysInfo.dwNumberOfProcessors);

  // Print threading capabilities
  //   Logger::log("Processor Type: ", sysInfo.dwProcessorType);
  //   Logger::log("Minimum Thread Count: ", sysInfo.dwNumberOfProcessors);
  //   Logger::log("Maximum Thread Count: ", sysInfo.dwNumberOfProcessors);
  //   Logger::log("Page Size: ", sysInfo.dwPageSize, " bytes");
  //   Logger::log("Minimum Application Address: ", sysInfo.lpMinimumApplicationAddress);
  //   Logger::log("Maximum Application Address: ", sysInfo.lpMaximumApplicationAddress);
  //   Logger::log("Active Processor Mask: ", sysInfo.dwActiveProcessorMask);
}