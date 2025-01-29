#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

#include "util/vector.h"

namespace util
{
class Transform
{
 public:
  Transform() = default;
  ~Transform() = default;

  const glm::mat4& matrix();

  Transform* parent() const
  {
    return _parent;
  }
  void setParent(Transform* parent);

  const glm::vec3& position() const
  {
    return _position;
  }
  void setPosition(const glm::vec3& position);

  const glm::quat& rotation() const
  {
    return _rotation;
  }
  void setRotation(const glm::quat& rotation);

  const glm::vec3& scale() const
  {
    return _scale;
  }
  void setScale(const glm::vec3& scale);

  const glm::vec3& origin() const
  {
    return _origin;
  }
  void setOrigin(const glm::vec3& origin);

 private:
  void markDirty();

  void recalculateMatrix();

 private:
  Transform* _parent = nullptr;
  std::vector<Transform*> _children;

  glm::mat4 _matrix{1.0f};

  glm::vec3 _position{0.0f};
  glm::quat _rotation{1.0f, 0.0f, 0.0f, 0.0f};
  glm::vec3 _scale{1.0f};

  glm::vec3 _origin{0.0f};

  bool _dirty = true;
};
}  // namespace util
