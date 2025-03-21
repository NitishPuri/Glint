### More examples of using builder for configuring structs

```cpp
auto pipeline = Pipeline::Builder(context, renderPass)
    .setShaders("shader.vert.spv", "shader.frag.spv")
    .setVertexFormat(VertexAttributeFlags::POSITION_TEXCOORD)
    .setCullMode(VK_CULL_MODE_BACK)
    .setBlending(true)
    .setDepthTest(true)
    .setDescriptorSetLayout(descriptorSetLayout.get())
    .build();
```

```cpp
auto renderPass = RenderPass::Builder(context)
    .addColorAttachment(swapChainFormat, VK_ATTACHMENT_LOAD_OP_CLEAR)
    .addDepthAttachment(depthFormat)
    .createSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
        .addColorReference(0)
        .addDepthReference(1)
        .done()
    .addDependency(/* dependency details */)
    .build();
```

```cpp
commandBuffer.begin()
    .beginRenderPass(renderPass, framebuffer, clearValues)
    .bindPipeline(pipeline)
    .bindDescriptorSets(pipelineLayout, descriptorSets)
    .setViewport(viewport)
    .setScissor(scissor)
    .bindVertexBuffers(vertexBuffers)
    .bindIndexBuffer(indexBuffer)
    .drawIndexed(indexCount)
    .endRenderPass()
    .end();
```

```cpp
auto entity = Entity::Builder()
    .setName("Player")
    .setPosition({0, 0, 0})
    .addComponent<MeshComponent>(mesh)
    .addComponent<MaterialComponent>(material)
    .addComponent<PhysicsComponent>()
        .setMass(80.0f)
        .setCollider(ColliderType::CAPSULE)
        .done()
    .build();
```
