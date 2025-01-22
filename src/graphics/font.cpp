#include "font.h"

#include <stb_image.h>

#include <fstream>
#include <iostream>
#include <vector>

namespace graphics
{
Font::Font(
  const wgpu::Device& device,
  const wgpu::Queue& queue,
  const std::filesystem::path& directory
)
{
  auto jsonPath = directory / directory.filename().concat(".json");

  std::ifstream file(jsonPath);
  if (!file.is_open())
  {
    throw std::runtime_error("Could not open " + jsonPath.string());
  }
  auto json = nlohmann::json::parse(file);

  for (const auto& c : json["chars"])
  {
    FontCharacter character = {
      .bounds =
        {
          .x = c["x"],
          .y = c["y"],
          .width = c["width"],
          .height = c["height"],
        },
      .offset = {c["xoffset"], c["yoffset"]},
      .page = c["page"],
      .advance = c["xadvance"],
    };
    _characters.insert({c["char"], character});
  }

  auto atlasPath = directory / json["pages"][0];

  int width, height, channels;
  uint8_t* pixels = stbi_load(
    atlasPath.string().c_str(),
    &width,
    &height,
    &channels,
    STBI_rgb_alpha
  );

  if (!pixels)
  {
    throw std::runtime_error("Failed to load image: " + atlasPath.string());
  }

  wgpu::TextureDescriptor textureDescriptor{};
  textureDescriptor.dimension = wgpu::TextureDimension::e2D;
  textureDescriptor.label = "Font Atlas";
  textureDescriptor.size = {(uint32_t)width, (uint32_t)height, 1};
  textureDescriptor.mipLevelCount = 1;
  textureDescriptor.sampleCount = 1;
  textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
  textureDescriptor.usage =
    wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
  _atlas = device.CreateTexture(&textureDescriptor);

  wgpu::ImageCopyTexture destination{};
  destination.texture = _atlas;
  destination.mipLevel = 0;
  destination.origin = {0, 0, 0};
  destination.aspect = wgpu::TextureAspect::All;

  wgpu::TextureDataLayout source{};
  source.offset = 0;
  source.bytesPerRow = 4 * sizeof(uint8_t) * width;
  source.rowsPerImage = height;

  queue.WriteTexture(
    &destination,
    pixels,
    4 * sizeof(uint8_t) * width * height,
    &source,
    &textureDescriptor.size
  );

  stbi_image_free(pixels);

  wgpu::TextureViewDescriptor viewDescriptor{};
  viewDescriptor.label = "Font Atlas View";
  viewDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
  viewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
  viewDescriptor.usage = wgpu::TextureUsage::TextureBinding;
  viewDescriptor.aspect = wgpu::TextureAspect::All;
  viewDescriptor.baseArrayLayer = 0;
  viewDescriptor.arrayLayerCount = 1;
  viewDescriptor.baseMipLevel = 0;
  viewDescriptor.mipLevelCount = 1;
  _atlasView = _atlas.CreateView(&viewDescriptor);
}
}  // namespace graphics
