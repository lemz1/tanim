#pragma once

#include <functional>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <string>

#include "font.h"

namespace graphics
{
enum class TextAlignment
{
  Left,
  Centered,
  Right,
};

struct TextCharacter
{
  alignas(16) glm::mat4 transform;
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

  const glm::vec3& position() const
  {
    return _position;
  }
  void setPosition(const glm::vec3& position);

  const glm::quat& rotation() const
  {
    return _rotation;
  }
  void setRotation(const glm::quat& rotation);

  const glm::vec3& scale() const
  {
    return _scale;
  }
  void setScale(const glm::vec3& scale);

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

  void recalculateOrigin();

  void recalculateTransform();

 private:
  TextAlignment _alignment = TextAlignment::Left;
  glm::vec3 _origin{0.0f};

  glm::mat4 _transform{1.0f};
  glm::vec3 _color{1.0f};

  glm::vec3 _position{0.0f};
  glm::quat _rotation{1.0f, 0.0f, 0.0f, 0.0f};
  glm::vec3 _scale{1.0f};

  std::string _text;
  std::reference_wrapper<const Font> _font;

  std::vector<TextCharacter> _characters;
  float _width;
  float _height;
};
}  // namespace graphics
