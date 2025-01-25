#include "text.h"

namespace graphics
{
static constexpr float scalingFactor = 0.01f;

Text::Text(
  const wgpu::Device& device,
  const wgpu::Queue& queue,
  std::string_view text,
  const Font& font
)
  : _device(device), _queue(queue), _text(text), _font(font)
{
  createBuffer();
  fillBuffer();
}
void Text::SetText(std::string_view text)
{
  if (_text == text)
  {
    return;
  }

  if (text.length() > _text.length())
  {
    createBuffer();
  }

  _text = text;
  fillBuffer();
}

void Text::SetFont(const Font& font)
{
  if (&_font.get() == &font)
  {
    return;
  }

  _font = font;
  fillBuffer();
}

void Text::createBuffer()
{
  wgpu::BufferDescriptor bufferDescriptor{};
  bufferDescriptor.label = "Text Character Buffer";
  bufferDescriptor.size = _text.length() * sizeof(TextCharacter);
  bufferDescriptor.usage =
    wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
  _characterBuffer = _device.CreateBuffer(&bufferDescriptor);
}

void Text::fillBuffer()
{
  std::vector<TextCharacter> textChars;
  textChars.reserve(_text.length());

  _characterCount = 0;
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

    textChars.emplace_back(textChar);

    cursor.x += fontChar.advance;
    _characterCount++;
  }

  _queue.WriteBuffer(
    _characterBuffer,
    0,
    textChars.data(),
    _characterCount * sizeof(TextCharacter)
  );
}
}  // namespace graphics
