#pragma once

#include <webgpu/webgpu_cpp.h>

#include <array>
#include <filesystem>
#include <glm/glm.hpp>

#include "camera.h"
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

  void drawQuad(float x, float y);

  void drawText(const Text& text, const Camera& camera);

  void flush(const wgpu::TextureView& view);

  const wgpu::Sampler& linearSampler() const
  {
    return _linearSampler;
  }

  const wgpu::Sampler& nearestSampler() const
  {
    return _nearestSampler;
  }

  const wgpu::RenderPipeline& textPipeline() const
  {
    return _textPipeline;
  }
  const Font& font(const std::filesystem::path& path);

 private:
  void createSamplers();

  void createTextBuffers();
  void createTextPipeline(wgpu::TextureFormat format);

  void createDrawBuffers();

 private:
  wgpu::Sampler _linearSampler;
  wgpu::Sampler _nearestSampler;

  wgpu::Buffer _textBuffer;
  wgpu::Buffer _textUniformBuffer;
  size_t _textBufferOffset = 0;
  wgpu::BindGroup _textBindGroup;
  wgpu::RenderPipeline _textPipeline;

  std::array<wgpu::VertexAttribute, 2> _vertexAttributes;
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
