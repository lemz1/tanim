#include "font.h"

#include <fstream>
#include <iostream>

namespace graphics
{
Font::Font(const std::filesystem::path& directory)
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
      .offset = glm::vec2(c["xoffset"], c["yoffset"]),
      .page = c["page"],
      .advance = c["xadvance"],
    };
    _characters.insert({c["char"], character});
  }
}
}  // namespace graphics
