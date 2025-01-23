#pragma once

#include <webgpu/webgpu_cpp.h>

#include <filesystem>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>

namespace graphics
{
using FontUnicode = uint32_t;

struct FontUVBounds
{
  float left;
  float right;
  float top;
  float bottom;
};

struct FontCharacter
{
  FontUVBounds bounds;
  glm::vec2 size;
  glm::vec2 offset;
  uint32_t page;
  int advance;
};

class Font
{
 public:
  Font(
    const wgpu::Device& device,
    const wgpu::Queue& queue,
    const std::filesystem::path& directory
  );
  ~Font() = default;

  const auto& operator[](FontUnicode unicode) const
  {
    return _characters.at(unicode);
  }

  const auto& Character(FontUnicode unicode) const
  {
    return _characters.at(unicode);
  }

  const auto& operator[](char c) const
  {
    return _characters.at((uint32_t)c);
  }

  const auto& Character(char c) const
  {
    return _characters.at((uint32_t)c);
  }

  const auto& Characters() const
  {
    return _characters;
  }

  const auto& Atlas() const
  {
    return _atlas;
  }

  const auto& AtlasView() const
  {
    return _atlasView;
  }

 private:
  std::unordered_map<FontUnicode, FontCharacter> _characters = {};
  wgpu::Texture _atlas;
  wgpu::TextureView _atlasView;
};
}  // namespace graphics
