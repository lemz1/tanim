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
  recalculateAlignment();
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
    character._data.color = color;
  }
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

    auto& textChar = _characters.emplace_back();
    textChar.transform.setParent(&transform);
    textChar._data.bounds = glm::vec4(
      fontChar.bounds.left,
      fontChar.bounds.right,
      fontChar.bounds.top,
      fontChar.bounds.bottom
    );
    textChar._data.size = fontChar.size;
    textChar._data.color = _color;
    textChar._data.position.x = cursor.x + fontChar.offset.x;
    textChar._data.position.y = cursor.y - fontChar.offset.y;

    if (i > 0)
    {
      textChar._data.position.x +=
        _font.get().kerning(_text.at(i - 1), _text.at(i));
    }

    textChar._data.size *= scalingFactor;
    textChar._data.position *= scalingFactor;

    textChar.transform.setOrigin(glm::vec3(
      textChar._data.position.x + textChar._data.size.x / 2.0f,
      textChar._data.position.y - textChar._data.size.y / 2.0f,
      0.0f
    ));

    _width =
      std::max(_width, textChar._data.position.x + textChar._data.size.x);
    _height =
      std::max(_height, -textChar._data.position.y + textChar._data.size.y);

    cursor.x += fontChar.advance;
  }

  recalculateOrigin();
  recalculateAlignment();
}

void Text::recalculateOrigin()
{
  float halfLineHeight = _font.get().lineHeight() * scalingFactor / 2.0f;

  switch (_alignment)
  {
    case TextAlignment::Left:
      transform.setOrigin(glm::vec3(0.0f, -halfLineHeight, 0.0f));
      break;

    case TextAlignment::Centered:
      transform.setOrigin(glm::vec3(_width / 2.0f, -halfLineHeight, 0.0f));
      break;

    case TextAlignment::Right:
      transform.setOrigin(glm::vec3(_width, -halfLineHeight, 0.0f));
      break;
  }
}

void Text::recalculateAlignment()
{
  glm::vec2 offset{0.0f};
  float halfLineHeight = _font.get().lineHeight() * scalingFactor / 2.0f;

  switch (_alignment)
  {
    case TextAlignment::Left:
      offset = glm::vec2(0.0f, halfLineHeight);
      break;

    case TextAlignment::Centered:
      offset = glm::vec2(-_width / 2.0f, halfLineHeight);
      break;

    case TextAlignment::Right:
      offset = glm::vec2(-_width, halfLineHeight);
      break;
  }

  for (auto& character : _characters)
  {
    character._offset = offset;
  }
}
}  // namespace graphics
