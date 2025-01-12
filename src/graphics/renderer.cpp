#include "renderer.h"

namespace graphics
{
constexpr size_t vertexBufferSize = 4096 * sizeof(float);
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
}

Renderer::~Renderer()
{
}

void Renderer::DrawQuad(float x, float y)
{
  std::vector<float> vertices = {
    -0.5f + x,
    -0.5f + y,
    +0.0f,
    +0.0f,
    //
    +0.5f + x,
    -0.5f + y,
    +1.0f,
    +0.0f,
    //
    -0.5f + x,
    +0.5f + y,
    +0.0f,
    +1.0f,
    //
    +0.5f + x,
    +0.5f + y,
    +1.0f,
    +1.0f,
  };
  _queue.WriteBuffer(
    _vertexBuffer,
    _vertexBufferOffset,
    vertices.data(),
    vertices.size() * sizeof(float)
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

  _vertexBufferOffset += vertices.size() * sizeof(float);
  _indexBufferOffset += indices.size() * sizeof(uint32_t);
  _indexValueOffset += 4;
}

void Renderer::Flush(
  const wgpu::TextureView& view,
  const wgpu::RenderPipeline& pipeline
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
