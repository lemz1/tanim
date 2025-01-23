#pragma once

#include <webgpu/webgpu_cpp.h>

#include <filesystem>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>

namespace graphics
{
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

  const auto& operator[](uint32_t unicode) const
  {
    return _characters.at(unicode);
  }

  const auto& Character(uint32_t unicode) const
  {
    return _characters.at(unicode);
  }

  const auto& Characters() const
  {
    return _characters;
  }

  auto Kerning(uint32_t firstUnicode, uint32_t secondUnicode) const
  {
    auto it = _kernings.find(KerningKey(firstUnicode, secondUnicode));
    if (it == _kernings.end())
    {
      return 0.0f;
    }
    return it->second;
  }

  const auto& Kernings() const
  {
    return _kernings;
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
  uint64_t KerningKey(uint32_t firstUnicode, uint32_t secondUnicode) const
  {
    return (uint64_t)firstUnicode << 32 | secondUnicode;
  }

 private:
  std::unordered_map<uint32_t, FontCharacter> _characters = {};
  std::unordered_map<uint64_t, float> _kernings = {};
  wgpu::Texture _atlas;
  wgpu::TextureView _atlasView;
};
}  // namespace graphics
