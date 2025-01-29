#pragma once

#include <vector>

namespace util
{
template <typename T>
void swapRemove(std::vector<T>& vec, size_t index)
{
  if (index >= vec.size())
  {
    return;
  }

  vec[index] = std::move(vec.back());
  vec.pop_back();
}
}  // namespace util
