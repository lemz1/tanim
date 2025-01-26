#include "text.h"

namespace graphics
{
constexpr float scalingFactor = 0.01f;

Text::Text(std::string_view text, const Font& font) : _text(text), _font(font)
{
  updateCharacters();
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
}
}  // namespace graphics
