#pragma once

#include <functional>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <string>

#include "graphics/font.h"
#include "graphics/gpu_types.h"
#include "util/transform.h"

namespace graphics
{
enum class TextAlignment
{
  Left,
  Centered,
  Right,
};

class TextCharacter
{
 public:
  const TextCharacterGPU& data()
  {
    _data.transform = transform.matrix();
    return _data;
  }

 public:
  util::Transform transform;

 private:
  TextCharacterGPU _data;

  friend class Text;
};

class Text
{
 public:
  Text(std::string_view text, const Font& font);
  ~Text() = default;

  TextAlignment alignment() const
  {
    return _alignment;
  }
  void setAlignment(TextAlignment alignment);

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

  const TextCharacter& character(size_t index) const
  {
    return _characters.at(index);
  }

  TextCharacter& character(size_t index)
  {
    return _characters.at(index);
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
  util::Transform transform;

 private:
  void updateCharacters();

  void recalculateOrigin();

 private:
  TextAlignment _alignment = TextAlignment::Left;

  glm::vec3 _color{1.0f};

  std::string _text;
  std::reference_wrapper<const Font> _font;

  std::vector<TextCharacter> _characters;
  float _width;
  float _height;

  friend class Renderer;
};
}  // namespace graphics
