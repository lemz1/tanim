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
  Text(std::string_view text, const Font& font);
  ~Text() = default;

  void SetText(std::string_view text);

  void SetFont(const Font& font);

  const std::string& text() const
  {
    return _text;
  }

  const Font& font() const
  {
    return _font;
  }

  const std::vector<TextCharacter>& characters() const
  {
    return _characters;
  }

  float width() const
  {
    return _width;
  }

  float height() const
  {
    return _height;
  }

 public:
  glm::vec3 color{1.0f};

 private:
  void updateCharacters();

 private:
  std::string _text;
  std::reference_wrapper<const Font> _font;

  std::vector<TextCharacter> _characters;
  float _width;
  float _height;
};
}  // namespace graphics
