#include "dynamic_uniform_buffer.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <glm/gtc/matrix_transform.hpp>
#include <random>

#include "core/config.h"
#include "core/logger.h"
#include "renderer/descriptor.h"
#include "renderer/initializers.h"
#include "renderer/mesh_factory.h"
#include "renderer/pipeline.h"
#include "renderer/render_pass.h"
#include "renderer/renderer.h"
#include "renderer/swapchain.h"
#include "renderer/vk_context.h"
#include "renderer/vk_utils.h"

// Wrapper functions for aligned memory allocation
// There is currently no standard for this in C++ that works across all platforms and vendors, so we abstract this
void* alignedAlloc(size_t size, size_t alignment) {
  void* data = nullptr;
#if defined(_MSC_VER) || defined(__MINGW32__)
  data = _aligned_malloc(size, alignment);
#else
  int res = posix_memalign(&data, alignment, size);
  if (res != 0) data = nullptr;
#endif
  return data;
}

void alignedFree(void* data) {
#if defined(_MSC_VER) || defined(__MINGW32__)
  _aligned_free(data);
#else
  free(data);
#endif
}

namespace glint {

// struct UniformBufferObject {
//   alignas(16) glm::mat4 model;
//   alignas(16) glm::mat4 view;
//   alignas(16) glm::mat4 proj;
// };

DynamicUniformBuffer::DynamicUniformBuffer() : Sample("DynamicUniformBuffer") { LOGFN; }

void DynamicUniformBuffer::prepareUniformBuffers() {
  LOGFN;
  // Allocate data for the dynamic uniform buffer object
  // We allocate this manually as the alignment of the offset differs between GPUs

  // Calculate required alignment based on minimum device offset alignment
  size_t minUboAlignment =
      m_Renderer->getContext()->getPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
  dynamicAlignment = sizeof(glm::mat4);
  if (minUboAlignment > 0) {
    dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
  }

  size_t bufferSize = OBJECT_INSTANCES * dynamicAlignment;

  uboDataDynamic.model = (glm::mat4*)alignedAlloc(bufferSize, dynamicAlignment);
  assert(uboDataDynamic.model);

  LOG("minUniformBufferOffsetAlignment = ", minUboAlignment);
  LOG("dynamicAlignment = ", dynamicAlignment);
  LOG("bufferSize = ", bufferSize);

  // Create the uniform buffer for the view and projection matrices
  m_viewUBO =
      std::make_unique<UniformBuffer>(m_Renderer->getContext(), sizeof(uboVS),
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  // Create the uniform buffer for the dynamic model matrix
  m_dynamicUBO =
      std::make_unique<UniformBuffer>(m_Renderer->getContext(), bufferSize, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

  // Override descriptor range from bufferSize to dynamicAlignment
  m_dynamicUBO->descriptor().range = dynamicAlignment;

  // Prepare per-object matrices with offsets and random rotations
  std::default_random_engine rndEngine((unsigned)time(nullptr));
  std::normal_distribution<float> rndDist(-1.0f, 1.0f);
  for (uint32_t i = 0; i < OBJECT_INSTANCES; i++) {
    rotations[i] = glm::vec3(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine)) * 2.0f * (float)M_PI;
    rotationSpeeds[i] = glm::vec3(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine));
  }

  // updateUniformBuffers(0);
  updateUniformBuffers();
  updateDynamicUniformBuffer();
}

void DynamicUniformBuffer::setupDescriptors() {
  auto device = m_Renderer->getContext()->getDevice();
  // Pool
  std::vector<VkDescriptorPoolSize> poolSizes = {
      glint::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
      // Dynamic uniform buffer
      glint::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1)};

  VkDescriptorPoolCreateInfo descriptorPoolInfo = glint::initializers::descriptorPoolCreateInfo(poolSizes, 2);
  VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));

  // Layout
  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
      glint::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
      // Dynamic uniform buffer
      glint::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                                      VK_SHADER_STAGE_VERTEX_BIT, 1)};

  VkDescriptorSetLayoutCreateInfo descriptorLayout =
      glint::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
  VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout));

  // Set
  VkDescriptorSetAllocateInfo allocInfo =
      glint::initializers::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);
  VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));

  // write descriptors
  // VkDescriptorBufferInfo viewUniformDesc{};
  // viewUniformDesc.buffer = m_viewUBO->getBuffer();
  // viewUniformDesc.offset = offset;
  // viewUniformDesc.range = size;

  std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
      // Binding 0 : Projection/View matrix as uniform buffer
      glint::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0,
                                              &m_viewUBO->descriptor()),
      // Binding 1 : Instance matrix as dynamic uniform buffer
      glint::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1,
                                              &m_dynamicUBO->descriptor())};

  vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0,
                         nullptr);

  // Update descriptor sets
}

void DynamicUniformBuffer::initSample(Window* window, Renderer* renderer) {
  uint32_t framesInFlight = renderer->getFramesInFlight();
  LOG("Creating resources for", framesInFlight, "frames in flight");

  m_Mesh = MeshFactory::createCube(renderer->getContext());

  initCamera(1, 60, 0.1, 256);
  m_Camera->setPosition(0.0f, 0.0f, -30.0f);

  // Create descriptor set layout
  // m_DescriptorSetLayout = DescriptorSetLayout::Builder(renderer->getContext())
  //                             .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
  //                             .addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT)
  //                             .build();

  // // Create descriptor pool with enough capacity for uniform buffers and
  // m_DescriptorPool = std::make_unique<DescriptorPool>(renderer->getContext(), m_DescriptorSetLayout.get(), 2);

  prepareUniformBuffers();
  setupDescriptors();

  // Create pipeline
  PipelineConfig config;
  // config.descriptorSetLayout = m_DescriptorSetLayout.get();
  config.descriptorSetLayout = descriptorSetLayout;
  config.vertexShaderPath = Config::getShaderFile("dynamic_uniform_buffer.vert");
  config.fragmentShaderPath = Config::getShaderFile("base.frag");
  config.vertexFormat = VertexAttributeFlags::POSITION_COLOR;
  config.depthTestEnable = true;
  config.depthWriteEnable = true;
  // config.cullMode = VK_CULL_MODE_BACK_BIT;
  config.cullMode = VK_CULL_MODE_NONE;

  renderer->createPipeline(&config);

  // Create uniform buffer for each frame in flight
  // m_UniformBuffers.resize(framesInFlight);
  // for (auto& ubo : m_UniformBuffers) {
  //   ubo = std::make_unique<UniformBuffer>(renderer->getContext(), sizeof(UniformBufferObject));
  // }

  // Create descriptor
  // m_Descriptor =
  //     std::make_unique<Descriptor>(renderer->getContext(), m_DescriptorSetLayout.get(), m_DescriptorPool.get(), 2);

  // TODO Can we move this with prepareUniformbuffers?
  // for (uint32_t i = 0; i < framesInFlight; i++) {
  // Update uniform buffer
  // m_Descriptor->updateUniformBuffer(0, m_viewUBO->getBuffer(), sizeof(uboVS), 0, 0);
  // m_Descriptor->updateUniformBuffer(m_dynamicUBO->getBuffer(), dynamicAlignment, 0, 0);
  auto bufferSize = OBJECT_INSTANCES * dynamicAlignment;
  // m_Descriptor->updateUniformBuffer(1, m_dynamicUBO->getBuffer(), bufferSize, 1, 0);

  // Initial transformations will be set in updateUniformBuffer
  updateUniformBuffers();
  updateDynamicUniformBuffer();
}

void DynamicUniformBuffer::update(float deltaTime) {
  // Update rotation angle
  m_RotationAngle += deltaTime * 45.0f;  // 45 degrees per second

  // Keep angle between 0-360 degrees
  if (m_RotationAngle > 360.0f) {
    m_RotationAngle -= 360.0f;
  }

  static auto t = deltaTime;
  t += deltaTime;
  auto f = std::sin(t);
  m_ModelPosition = glm::vec3(f / 2, 0.0f, 0.0f);
  m_RotationAxis = glm::mix(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), f);

  processCameraInput();
  updateCamera(deltaTime);

  // Get current frame index
  uint32_t currentFrame = m_Renderer->getCurrentFrame();

  animationTimer += deltaTime;
  // Update the uniform buffer with new transformation
  // updateUniformBuffer(currentFrame);
  updateUniformBuffers();
  updateDynamicUniformBuffer();
}

// void DynamicUniformBuffer::updateUniformBuffers(uint32_t currentImage) {
//   // UniformBufferObject ubo{};

//   // View matrix - slight distance from the quad
//   uboVS.view = m_Camera->getViewMatrix();
//   uboVS.projection = m_Camera->getProjectionMatrix();

//   // Update the uniform buffer
//   // m_UniformBuffers[currentImage]->update(&ubo, sizeof(ubo));
//   m_viewUBO->update(&uboVS, sizeof(uboVS));

//   // Update dynamic uniform buffer
//   // Update at max. 60 fps
//   // animationTimer += frameTimer;
//   if (animationTimer <= 1.0f / 60.0f) {
//     return;
//   }

//   // Dynamic ubo with per-object model matrices indexed by offsets in the command buffer
//   uint32_t dim = static_cast<uint32_t>(pow(OBJECT_INSTANCES, (1.0f / 3.0f)));
//   glm::vec3 offset(5.0f);

//   for (uint32_t x = 0; x < dim; x++) {
//     for (uint32_t y = 0; y < dim; y++) {
//       for (uint32_t z = 0; z < dim; z++) {
//         uint32_t index = x * dim * dim + y * dim + z;

//         // Aligned offset
//         glm::mat4* modelMat = (glm::mat4*)(((uint64_t)uboDataDynamic.model + (index * dynamicAlignment)));

//         // Update rotations
//         rotations[index] += animationTimer * rotationSpeeds[index];

//         // Update matrices
//         glm::vec3 pos = glm::vec3(-((dim * offset.x) / 2.0f) + offset.x / 2.0f + x * offset.x,
//                                   -((dim * offset.y) / 2.0f) + offset.y / 2.0f + y * offset.y,
//                                   -((dim * offset.z) / 2.0f) + offset.z / 2.0f + z * offset.z);
//         *modelMat = glm::translate(glm::mat4(1.0f), pos);
//         *modelMat = glm::rotate(*modelMat, rotations[index].x, glm::vec3(1.0f, 1.0f, 0.0f));
//         *modelMat = glm::rotate(*modelMat, rotations[index].y, glm::vec3(0.0f, 1.0f, 0.0f));
//         *modelMat = glm::rotate(*modelMat, rotations[index].z, glm::vec3(0.0f, 0.0f, 1.0f));
//       }
//     }
//   }

//   animationTimer = 0.0f;

//   auto bufferSize = OBJECT_INSTANCES * dynamicAlignment;
//   m_dynamicUBO->update(uboDataDynamic.model, bufferSize);

//   auto device = m_Renderer->getContext()->getDevice();
//   // memcpy(uniformBuffers.dynamic.mapped, uboDataDynamic.model, uniformBuffers.dynamic.size);
//   // Flush to make changes visible to the host
//   VkMappedMemoryRange memoryRange;
//   memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
//   memoryRange.pNext = nullptr;
//   memoryRange.memory = m_dynamicUBO->getMemory();
//   memoryRange.size = vkFlushMappedMemoryRanges(device, 1, &memoryRange);
// }

void DynamicUniformBuffer::updateUniformBuffers() {
  LOGFN_ONCE;
  // Fixed ubo with projection and view matrices
  uboVS.projection = m_Camera->getProjectionMatrix();
  uboVS.view = m_Camera->getViewMatrix();
  m_viewUBO->update(&uboVS);
}

void DynamicUniformBuffer::updateDynamicUniformBuffer() {
  assert(m_Renderer->getContext());
  auto device = m_Renderer->getContext()->getDevice();

  // Update at max. 60 fps
  // animationTimer += frameTimer;
  if (animationTimer <= 1.0f / 60.0f) {
    return;
  }

  // Dynamic ubo with per-object model matrices indexed by offsets in the command buffer
  uint32_t dim = static_cast<uint32_t>(pow(OBJECT_INSTANCES, (1.0f / 3.0f)));
  glm::vec3 offset(5.0f);

  for (uint32_t x = 0; x < dim; x++) {
    for (uint32_t y = 0; y < dim; y++) {
      for (uint32_t z = 0; z < dim; z++) {
        uint32_t index = x * dim * dim + y * dim + z;

        // Aligned offset
        glm::mat4* modelMat = (glm::mat4*)(((uint64_t)uboDataDynamic.model + (index * dynamicAlignment)));

        // Update rotations
        rotations[index] += animationTimer * rotationSpeeds[index];

        // Update matrices
        glm::vec3 pos = glm::vec3(-((dim * offset.x) / 2.0f) + offset.x / 2.0f + x * offset.x,
                                  -((dim * offset.y) / 2.0f) + offset.y / 2.0f + y * offset.y,
                                  -((dim * offset.z) / 2.0f) + offset.z / 2.0f + z * offset.z);
        *modelMat = glm::translate(glm::mat4(1.0f), pos);
        *modelMat = glm::rotate(*modelMat, rotations[index].x, glm::vec3(1.0f, 1.0f, 0.0f));
        *modelMat = glm::rotate(*modelMat, rotations[index].y, glm::vec3(0.0f, 1.0f, 0.0f));
        *modelMat = glm::rotate(*modelMat, rotations[index].z, glm::vec3(0.0f, 0.0f, 1.0f));
      }
    }
  }

  animationTimer = 0.0f;

  // memcpy(uniformBuffers.dynamic.mapped, uboDataDynamic.model, uniformBuffers.dynamic.size);
  m_dynamicUBO->update(uboDataDynamic.model);

  // Flush to make changes visible to the host
  // TODO: m_dynamicUBO->flush() instead.
  VkMappedMemoryRange memoryRange = glint::initializers::mappedMemoryRange();
  memoryRange.memory = m_dynamicUBO->getMemory();
  memoryRange.size = m_dynamicUBO->getSize();
  vkFlushMappedMemoryRanges(device, 1, &memoryRange);
}

void DynamicUniformBuffer::render(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
  auto pipeline = m_Renderer->getPipeline();
  uint32_t currentFrame = m_Renderer->getCurrentFrame();

  // Bind pipeline and descriptor sets
  pipeline->bind(commandBuffer);
  // m_Descriptor->bind(commandBuffer, pipeline->getPipelineLayout(), currentFrame);

  m_Mesh->bind(commandBuffer);

  // Set viewport and scissor
  setupDefaultVieportAndScissor(commandBuffer, m_Renderer);

  // m_Descriptor->bind(commandBuffer, pipeline->getPipelineLayout(), 0);

  // Bind and draw mesh
  // m_Mesh->draw(commandBuffer);
  for (uint32_t j = 0; j < OBJECT_INSTANCES; j++) {
    // One dynamic offset per dynamic descriptor to offset into the ubo containing all model matrices
    uint32_t dynamicOffset = j * static_cast<uint32_t>(dynamicAlignment);
    // Bind the descriptor set for rendering a mesh using the dynamic offset
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayout(), 0, 1,
                            &descriptorSet, 1, &dynamicOffset);

    // vkCmdDrawIndexed(drawCmdBuffers[i], indexCount, 1, 0, 0, 0);
    m_Mesh->draw(commandBuffer);
  }

  // uint32_t dim = static_cast<uint32_t>(pow(OBJECT_INSTANCES, (1.0f / 3.0f)));
  // for (uint32_t x = 0; x < dim; x++) {
  //   for (uint32_t y = 0; y < dim; y++) {
  //     for (uint32_t z = 0; z < dim; z++) {
  //       uint32_t index = x * dim * dim + y * dim + z;

  //       // Calculate dynamic offset for this instance
  //       uint32_t dynamicOffset = index * static_cast<uint32_t>(dynamicAlignment);

  //       // Bind descriptor set with dynamic offset
  //       // m_Descriptor->bind(commandBuffer, pipeline->getPipelineLayout(), 0, &dynamicOffset);

  //       // Draw the mesh
  //       m_Mesh->draw(commandBuffer);
  //     }
  //   }
  // }
}

void DynamicUniformBuffer::cleanup() {
  LOGFN;
  // Let RAII handle the resources

  // m_Descriptor.reset();
  // m_DescriptorPool.reset();
  // m_DescriptorSetLayout.reset();

  // Clean up uniform buffers
  // for (auto& ubo : m_UniformBuffers) {
  //   ubo.reset();
  // }
  // m_UniformBuffers.clear();
  m_viewUBO.reset();
  m_dynamicUBO.reset();

  if (uboDataDynamic.model) {
    alignedFree(uboDataDynamic.model);
    // free(uboDataDynamic.model);
    uboDataDynamic.model = nullptr;
  }

  m_Mesh.reset();
}

REGISTER_SAMPLE(DynamicUniformBuffer);

}  // namespace glint