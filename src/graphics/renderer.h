#pragma once

#include <webgpu/webgpu_cpp.h>

#include <array>
#include <glm/glm.hpp>

namespace graphics
{
struct Vertex
{
  glm::vec3 position;
  glm::vec2 texCoord;
};

class Renderer
{
 public:
  Renderer(const wgpu::Device& device);
  ~Renderer();

  void DrawQuad(float x, float y);

  void Flush(
    const wgpu::TextureView& view,
    const wgpu::RenderPipeline& pipeline,
    const wgpu::BindGroup& bindGroup
  );

  const wgpu::VertexState& VertexState() const
  {
    return _vertexState;
  }

 private:
  std::array<wgpu::VertexAttribute, 2> _vertexAttributes;
  wgpu::VertexBufferLayout _vertexBufferLayout;
  wgpu::ShaderModule _shaderModule;
  wgpu::VertexState _vertexState;

  wgpu::Buffer _vertexBuffer;
  wgpu::Buffer _indexBuffer;

  size_t _vertexBufferOffset = 0;
  size_t _indexBufferOffset = 0;
  uint32_t _indexValueOffset = 0;

  const wgpu::Device& _device;
  wgpu::Queue _queue;
};
}  // namespace graphics
