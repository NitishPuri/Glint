# Glint

![alt text](<doc/Glint 2025-03-19 02.09.21.excalidraw.svg>)

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
│ │ ├── vk_context.h # Vulkan instance, device, etc.
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

## Next Steps

- Integrate Imgui
- Camera system
- Input handling
- Materials
- Basic lighting : diffuse, specular, point lights, spot lights
- skybox, cube maps
- normal mapping
- billboarding, geometry shaders
- particle system
- 3d picking
- tesselation
- instanced rendering
- deferred shading
- animation
- Basic profiling
- procedural meshes
- stencil shadow volume
- motion blur
- pcf
- ssao
- gpu gems
- bgfx examples
- dilligent engine examples
- Unit/Image based Tests
- Hot reload shaders
- Multiple materials
- Shadow mapping
- Post processing
- transpparency, blending
- scene graph
- ecs
- spatial partitioninng

### dependencies

- Vulkan SDK
- GLFW
- STB
- TinyObjloader
- Dear ImGui
