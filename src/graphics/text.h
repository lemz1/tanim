#pragma once

#include <functional>
#include <string>

#include "font.h"

namespace graphics
{
struct TextCharacter
{
  alignas(16) FontUVBounds bounds;
  alignas(16) glm::vec3 color;
  alignas(8) glm::vec2 size;
  alignas(8) glm::vec2 position;
};

class Text
{
 public:
  Text(std::string_view text, const Font& font);
  ~Text() = default;

  const glm::vec3& color() const
  {
    return _color;
  }
  void setColor(const glm::vec3& color);

  const std::string& text() const
  {
    return _text;
  }
  void setText(std::string_view text);

  const Font& font() const
  {
    return _font;
  }
  void setFont(const Font& font);

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

 private:
  void updateCharacters();

 private:
  glm::vec3 _color{1.0f};

  std::string _text;
  std::reference_wrapper<const Font> _font;

  std::vector<TextCharacter> _characters;
  float _width;
  float _height;
};
}  // namespace graphics
