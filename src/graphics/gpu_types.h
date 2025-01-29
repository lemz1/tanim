#pragma once

#include <glm/glm.hpp>

namespace graphics
{
struct TextCharacterGPU
{
  alignas(16) glm::mat4 transform;
  alignas(16) glm::vec4 bounds;
  alignas(16) glm::vec3 color;
  alignas(8) glm::vec2 size;
  alignas(8) glm::vec2 position;
};
};  // namespace graphics
