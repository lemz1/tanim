#pragma once

#include <webgpu/webgpu_cpp.h>

#include <filesystem>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

namespace graphics
{
struct FontBounds
{
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;
};

struct FontCharacter
{
  FontBounds bounds;
  glm::ivec2 offset;
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

  const auto& operator[](const std::string& character) const
  {
    return _characters.at(character);
  }

  const auto& Character(const std::string& character) const
  {
    return _characters.at(character);
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
  std::unordered_map<std::string, FontCharacter> _characters = {};
  wgpu::Texture _atlas;
  wgpu::TextureView _atlasView;
};
}  // namespace graphics
