#include "renderer.h"

namespace graphics
{
constexpr size_t vertexBufferSize = 1000 * sizeof(Vertex);
constexpr size_t indexBufferSize = vertexBufferSize * 2;

Renderer::Renderer(const wgpu::Device& device) : _device(device)
{
  _queue = _device.GetQueue();

  wgpu::BufferDescriptor vertexBufferDescriptor{};
  vertexBufferDescriptor.label = "Renderer Vertex Buffer";
  vertexBufferDescriptor.size = vertexBufferSize;
  vertexBufferDescriptor.usage =
    wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
  _vertexBuffer = _device.CreateBuffer(&vertexBufferDescriptor);

  wgpu::BufferDescriptor indexBufferDescriptor{};
  indexBufferDescriptor.label = "Renderer Index Buffer";
  indexBufferDescriptor.size = indexBufferSize;
  indexBufferDescriptor.usage =
    wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
  _indexBuffer = _device.CreateBuffer(&indexBufferDescriptor);

  _vertexAttributes[0].format = wgpu::VertexFormat::Float32x3;
  _vertexAttributes[0].offset = offsetof(Vertex, position);
  _vertexAttributes[0].shaderLocation = 0;
  _vertexAttributes[1].format = wgpu::VertexFormat::Float32x2;
  _vertexAttributes[1].offset = offsetof(Vertex, texCoord);
  _vertexAttributes[1].shaderLocation = 1;

  _vertexBufferLayout.attributeCount = _vertexAttributes.size();
  _vertexBufferLayout.attributes = _vertexAttributes.data();
  _vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;
  _vertexBufferLayout.arrayStride = sizeof(Vertex);

  const char* shaderCode = R"(
    struct VertexInput {
      @location(0) position: vec3f,
      @location(1) texCoord: vec2f,
    };

    struct VertexOutput {
      @builtin(position) position: vec4f,
      @location(0) texCoord: vec2f,
    };

    @vertex fn vsMain(in: VertexInput) -> VertexOutput {
      var out: VertexOutput;
      out.position = vec4f(in.position, 1.0);
      out.texCoord = in.texCoord;
      return out;
    }
)";

  wgpu::ShaderModuleWGSLDescriptor wgslDescriptor{};
  wgslDescriptor.code = shaderCode;
  wgslDescriptor.sType = wgpu::SType::ShaderSourceWGSL;

  wgpu::ShaderModuleDescriptor shaderModuleDescriptor{};
  shaderModuleDescriptor.label = "Renderer Vertex Module";
  shaderModuleDescriptor.nextInChain = &wgslDescriptor;

  _shaderModule = _device.CreateShaderModule(&shaderModuleDescriptor);

  _vertexState.module = _shaderModule;
  _vertexState.bufferCount = 1;
  _vertexState.buffers = &_vertexBufferLayout;
}

Renderer::~Renderer()
{
}

void Renderer::DrawQuad(float x, float y)
{
  std::vector<Vertex> vertices = {
    {
      {-0.5f + x, -0.5f + y, +0.0f},
      {+0.0f, +0.0f},
    },
    {
      {+0.5f + x, -0.5f + y, +0.0f},
      {+1.0f, +0.0f},
    },
    {
      {-0.5f + x, +0.5f + y, +0.0f},
      {+0.0f, +1.0f},
    },
    {
      {+0.5f + x, +0.5f + y, +0.0f},
      {+1.0f, +1.0f},
    },
  };
  _queue.WriteBuffer(
    _vertexBuffer,
    _vertexBufferOffset,
    vertices.data(),
    vertices.size() * sizeof(Vertex)
  );

  std::vector<uint32_t> indices = {
    0 + _indexValueOffset,
    1 + _indexValueOffset,
    2 + _indexValueOffset,
    1 + _indexValueOffset,
    3 + _indexValueOffset,
    2 + _indexValueOffset
  };

  _queue.WriteBuffer(
    _indexBuffer,
    _indexBufferOffset,
    indices.data(),
    indices.size() * sizeof(uint32_t)
  );

  _vertexBufferOffset += vertices.size() * sizeof(Vertex);
  _indexBufferOffset += indices.size() * sizeof(uint32_t);
  _indexValueOffset += 4;
}

void Renderer::Flush(
  const wgpu::TextureView& view,
  const wgpu::RenderPipeline& pipeline,
  const wgpu::BindGroup& bindGroup
)
{
  wgpu::CommandEncoderDescriptor encoderDescriptor{};
  encoderDescriptor.label = "Renderer Command Encoder";
  auto encoder = _device.CreateCommandEncoder(&encoderDescriptor);

  wgpu::RenderPassColorAttachment colorAttachment{};
  colorAttachment.view = view;
  colorAttachment.loadOp = wgpu::LoadOp::Clear;
  colorAttachment.storeOp = wgpu::StoreOp::Store;
  colorAttachment.clearValue = {0.1, 0.1, 0.1, 1.0};
  colorAttachment.depthSlice = wgpu::kDepthSliceUndefined;

  wgpu::RenderPassDescriptor renderPassDescriptor{};
  renderPassDescriptor.label = "Renderer Render Pass";
  renderPassDescriptor.colorAttachmentCount = 1;
  renderPassDescriptor.colorAttachments = &colorAttachment;

  auto renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
  renderPass.SetPipeline(pipeline);
  renderPass.SetVertexBuffer(0, _vertexBuffer, 0, _vertexBufferOffset);
  renderPass.SetIndexBuffer(
    _indexBuffer,
    wgpu::IndexFormat::Uint32,
    0,
    _indexBufferOffset
  );
  renderPass.SetBindGroup(0, bindGroup);
  renderPass.DrawIndexed(_indexBufferOffset / sizeof(uint32_t), 1, 0, 0, 0);
  renderPass.End();

  wgpu::CommandBufferDescriptor commandDescriptor{};
  commandDescriptor.label = "Renderer Command Buffer";
  auto command = encoder.Finish(&commandDescriptor);

  _queue.Submit(1, &command);

  _vertexBufferOffset = 0;
  _indexBufferOffset = 0;
  _indexValueOffset = 0;
}
}  // namespace graphics
