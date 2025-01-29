#pragma once

#include <webgpu/webgpu_cpp.h>

#include <array>
#include <filesystem>
#include <glm/glm.hpp>

#include "graphics/camera.h"
#include "graphics/font.h"
#include "graphics/gpu_types.h"
#include "graphics/text.h"

namespace graphics
{
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

  void drawText(Text& text, const Camera& camera);

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

  std::vector<TextCharacterGPU> _textCharacterData;
  wgpu::Buffer _textCharacterBuffer;
  wgpu::Buffer _textUniformBuffer;
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
