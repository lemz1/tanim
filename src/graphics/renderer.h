#pragma once

#include <webgpu/webgpu_cpp.h>

#include <array>
#include <filesystem>
#include <glm/glm.hpp>

#include "font.h"
#include "text.h"

namespace graphics
{
struct Vertex
{
  glm::vec3 position;
  glm::vec2 uv;
};

class Renderer
{
 public:
  Renderer(
    const wgpu::Device& device,
    const wgpu::Queue& queue,
    wgpu::TextureFormat format
  );
  ~Renderer() = default;

  void DrawQuad(float x, float y);

  void DrawText(
    const graphics::Text& text,
    const wgpu::TextureView& view,
    const wgpu::BindGroup& vertexGroup,
    const wgpu::BindGroup& fragmentGroup
  );

  void Flush(
    const wgpu::TextureView& view,
    const wgpu::RenderPipeline& pipeline,
    const wgpu::BindGroup& bindGroup
  );

  const wgpu::Sampler& LinearSampler() const
  {
    return _linearSampler;
  }

  const wgpu::Sampler& NearestSampler() const
  {
    return _nearestSampler;
  }

  const wgpu::BindGroupLayout& TextVertexBindGroupLayout() const
  {
    return _textBindGroupLayouts[0];
  }

  const wgpu::BindGroupLayout& TextFragmentBindGroupLayout() const
  {
    return _textBindGroupLayouts[1];
  }

  const wgpu::RenderPipeline& TextPipeline() const
  {
    return _textPipeline;
  }
  const graphics::Font& Font(const std::filesystem::path& path);

 private:
  void CreateSamplers();

  void CreateTextPipeline(wgpu::TextureFormat format);

  void CreateDrawBuffers();

 private:
  wgpu::Sampler _linearSampler;
  wgpu::Sampler _nearestSampler;

  std::array<wgpu::VertexAttribute, 2> _vertexAttributes;
  std::array<wgpu::BindGroupLayout, 2> _textBindGroupLayouts;
  wgpu::RenderPipeline _textPipeline;

  wgpu::Buffer _vertexBuffer;
  wgpu::Buffer _indexBuffer;

  size_t _vertexBufferOffset = 0;
  size_t _indexBufferOffset = 0;
  uint32_t _indexValueOffset = 0;

  std::unordered_map<std::filesystem::path, graphics::Font> _fonts;

  const wgpu::Device& _device;
  const wgpu::Queue& _queue;
};
}  // namespace graphics
