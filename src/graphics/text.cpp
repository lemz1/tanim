#include "text.h"

namespace graphics
{
Text::Text(
  const wgpu::Device& device,
  const wgpu::Queue& queue,
  std::string_view text,
  const Font& font
)
  : _device(device), _queue(queue), _text(text), _font(font)
{
  CreateBuffer();
  FillBuffer();
}
void Text::SetText(std::string_view text)
{
  if (_text == text)
  {
    return;
  }

  if (text.length() > _text.length())
  {
    CreateBuffer();
  }

  _text = text;
  FillBuffer();
}

void Text::SetFont(const Font& font)
{
  if (&_font.get() == &font)
  {
    return;
  }

  _font = font;
  FillBuffer();
}

void Text::CreateBuffer()
{
  wgpu::BufferDescriptor bufferDescriptor{};
  bufferDescriptor.label = "Text Character Buffer";
  bufferDescriptor.size = _text.length() * sizeof(TextCharacter);
  bufferDescriptor.usage =
    wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
  _characterBuffer = _device.CreateBuffer(&bufferDescriptor);
}

void Text::FillBuffer()
{
  std::vector<TextCharacter> textChars(_text.length());

  float cursor = 0;
  for (size_t i = 0; i < _text.length(); i++)
  {
    auto& fontChar = _font.get().Character(_text.at(i));

    auto& textChar = textChars.at(i);
    textChar.bounds = fontChar.bounds;
    textChar.size = fontChar.size;
    textChar.position.x = cursor + fontChar.offset.x;
    textChar.position.y = fontChar.offset.y;

    if (i > 0)
    {
      textChar.position.x += _font.get().Kerning(_text.at(i - 1), _text.at(i));
    }

    cursor += fontChar.advance;
  }

  _queue.WriteBuffer(
    _characterBuffer,
    0,
    textChars.data(),
    textChars.size() * sizeof(TextCharacter)
  );
}
}  // namespace graphics
