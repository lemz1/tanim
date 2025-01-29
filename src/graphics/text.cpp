#include "text.h"

namespace graphics
{
constexpr float scalingFactor = 0.01f;

Text::Text(std::string_view text, const Font& font) : _text(text), _font(font)
{
  updateCharacters();
}

void Text::setAlignment(TextAlignment alignment)
{
  if (_alignment == alignment)
  {
    return;
  }

  _alignment = alignment;
  recalculateOrigin();
  recalculateTransform();
}

void Text::setColor(const glm::vec3& color)
{
  if (_color == color)
  {
    return;
  }

  _color = color;
  for (auto& character : _characters)
  {
    character.color = color;
  }
}

void Text::setPosition(const glm::vec3& position)
{
  if (_position == position)
  {
    return;
  }

  _position = position;
  recalculateTransform();
}

void Text::setRotation(const glm::quat& rotation)
{
  if (_rotation == rotation)
  {
    return;
  }

  _rotation = rotation;
  recalculateTransform();
}

void Text::setScale(const glm::vec3& scale)
{
  if (_scale == scale)
  {
    return;
  }

  _scale = scale;
  recalculateTransform();
}

void Text::setText(std::string_view text)
{
  if (_text == text)
  {
    return;
  }

  _text = text;
  updateCharacters();
}

void Text::setFont(const Font& font)
{
  if (&_font.get() == &font)
  {
    return;
  }

  _font = font;
  updateCharacters();
}

void Text::updateCharacters()
{
  _characters.clear();
  _characters.reserve(_text.length());

  _width = 0.0f;
  _height = 0.0f;
  glm::vec2 cursor{0, 0};
  for (size_t i = 0; i < _text.length(); i++)
  {
    if (_text.at(i) == ' ')
    {
      cursor.x += _font.get().character(' ').advance;
      continue;
    }
    else if (_text.at(i) == '\n')
    {
      cursor.x = 0;
      cursor.y -= _font.get().lineHeight();
      continue;
    }

    auto& fontChar = _font.get().character(_text.at(i));

    TextCharacter textChar{};
    textChar.bounds = fontChar.bounds;
    textChar.size = fontChar.size;
    textChar.color = _color;
    textChar.position.x = cursor.x + fontChar.offset.x;
    textChar.position.y = cursor.y - fontChar.offset.y;

    if (i > 0)
    {
      textChar.position.x += _font.get().kerning(_text.at(i - 1), _text.at(i));
    }

    textChar.size *= scalingFactor;
    textChar.position *= scalingFactor;

    _width = std::max(_width, textChar.position.x + textChar.size.x);
    _height = std::max(_height, -textChar.position.y + textChar.size.y);

    _characters.emplace_back(textChar);

    cursor.x += fontChar.advance;
  }

  recalculateOrigin();
  recalculateTransform();
}

void Text::recalculateOrigin()
{
  float originY = _font.get().lineHeight() * scalingFactor / 2.0f;

  switch (_alignment)
  {
    case TextAlignment::Left:
      _origin = glm::vec3(0.0f, originY, 0.0f);
      break;

    case TextAlignment::Centered:
      _origin = glm::vec3(-_width / 2.0f, originY, 0.0f);
      break;

    case TextAlignment::Right:
      _origin = glm::vec3(-_width, originY, 0.0f);
      break;
  }
}

void Text::recalculateTransform()
{
  _transform = glm::translate(glm::mat4(1.0f), _position);
  _transform *= glm::mat4_cast(_rotation);
  _transform = glm::scale(_transform, _scale);

  for (auto& character : _characters)
  {
    character.transform = _transform * glm::translate(glm::mat4(1.0f), _origin);
  }
}
}  // namespace graphics
