#pragma once

#include <functional>
#include <string>

#include "font.h"

namespace graphics
{
struct TextCharacter
{
  FontUVBounds bounds;
  glm::vec2 size;
  glm::vec2 position;
};

class Text
{
 public:
  Text(
    const wgpu::Device& device,
    const wgpu::Queue& queue,
    std::string_view text,
    const Font& font
  );
  ~Text() = default;

  void SetText(std::string_view text);

  void SetFont(const Font& font);

  const std::string& _Text() const
  {
    return _text;
  }

  const Font& _Font() const
  {
    return _font;
  }

  const wgpu::Buffer& Buffer() const
  {
    return _characterBuffer;
  }

  size_t CharacterCount() const
  {
    return _characterCount;
  }

 private:
  void CreateBuffer();
  void FillBuffer();

 private:
  const wgpu::Device& _device;
  const wgpu::Queue& _queue;

  std::string _text;
  std::reference_wrapper<const Font> _font;
  wgpu::Buffer _characterBuffer;

  size_t _characterCount;
};
}  // namespace graphics
