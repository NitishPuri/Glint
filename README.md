# Glint

```
Glint_vk/
├── src/ # Core engine source code
│ ├── core/ # Core engine systems
│ │ ├── application.h # Application management
│ │ ├── application.cpp
│ │ ├── logger.h # logging system
│ │ ├── logger.cpp
│ │ ├── window.h # GLFW window handling
│ │ └── window.cpp
│ ├── renderer/ # Vulkan renderer
│ │ ├── vulkan_context.h # Vulkan instance, device, etc.
│ │ ├── vulkan_context.cpp
│ │ ├── swapchain.h # Swapchain management
│ │ ├── swapchain.cpp
│ │ ├── pipeline.h # Graphics pipeline
│ │ ├── pipeline.cpp
│ │ ├── command.h # Command buffers and pools
│ │ ├── command.cpp
│ │ ├── render_pass.h # Render passes
│ │ ├── render_pass.cpp
│ │ ├── synchronization.h # Fences and semaphores
│ │ └── synchronization.cpp
│ ├── shaders/ # Shader source code
│ │ ├── compile.bat # Shader compilation scripts
│ │ ├── compile.sh
│ │ ├── shader.vert # Vertex shader
│ │ └── shader.frag # Fragment shader
│ └── main.cpp # Entry point
├── samples/ # Different sample applications
│ ├── hello_triangle/ # hello triangle
│ ├── texture_sample/ # Future texture sample
│ └── ...
├── include/ # Public API headers
│ └── glint/ # engine namespace
├── build/ # Build artifacts
├── bin/ # Compiled binaries
│ └── shaders/ # Compiled shaders
├── third_party/ # Third-party dependencies
│ ├── glfw/
│ └── ...
├── assets/ # Textures, models, etc.
├── CMakeLists.txt # CMake build system
└── README.md # Project documentation
```
