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

  const auto& GetText() const
  {
    return _text;
  }

  auto GetFont() const
  {
    return _font;
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
};
}  // namespace graphics

// TODO:

/* Result from the below comments

When the text has a length bigger than the amount of characters in the font
atlas, we should use seperate buffers, else we use a single buffer

*/

/* Single Buffer
sizeof = 32
struct CombinedData {
  bounds: vec4f,
  size: vec2f,
  offset_position: vec2f,
};
*/

/* Seperated Buffers

sizeof = 20
struct FontChar {
  bounds: vec4f,
  size: vec2f,
  offset: vec2f,
};

sizeof = 12
struct TextChar {
  chars: array<vec3f>,
};

*/

/* Function

These are linear functions:
f(x) = a * x + b;

SingleBuffer(numCharacters) = sizeof(CombinedData) * numCharacters;

SeperatedBuffers(numCharacters) = sizeof(TextChar) * numCharacters
                                + sizeof(FontChar) * numCharactersInAtlas

SingleBuffer(numCharacters) = SeperatedBuffers(numCharacters)

sizeof(CombinedData) * numCharacters = sizeof(TextChar) * numCharacters
                                     + sizeof(FontChar) * numCharactersInAtlas

sizeof(CombinedData) * numCharacters - sizeof(TextChar) * numCharacters
    = sizeof(FontChar) * numCharactersInAtlas

numCharacters = sizeof(FontChar) * numCharactersInAtlas
              / (sizeof(CombinedData) - sizeof(TextChar))
*/

/* Result

When the text has a length bigger than the amount of characters in the font
atlas, we should use seperate buffers, else we use a single buffer

*/
