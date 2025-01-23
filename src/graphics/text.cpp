#include "text.h"

namespace graphics
{
Text::Text(
  const wgpu::Device& device,
  const wgpu::Queue& queue,
  std::string_view text,
  const Font* font
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

void Text::SetFont(const Font* font)
{
  if (_font == font)
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
  float cursor = 0;
  for (auto c : _text)
  {
    auto& fontChar = _font->Character(c);

    TextCharacter textChar{};
    textChar.bounds = fontChar.bounds;
    textChar.size = fontChar.size;
    textChar.position.x = cursor + fontChar.offset.x;
    textChar.position.y = fontChar.offset.y;

    cursor += fontChar.advance;
  }
}
}  // namespace graphics
