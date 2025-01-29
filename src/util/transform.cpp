#include "transform.h"

#include <iostream>

namespace util
{
const glm::mat4& Transform::matrix()
{
  if (_dirty)
  {
    _dirty = false;
    recalculateMatrix();
  }

  return _matrix;
}

void Transform::setParent(Transform* parent)
{
  if (_parent == parent)
  {
    return;
  }

  if (_parent)
  {
    auto& siblings = _parent->_children;

    auto it = std::find(siblings.begin(), siblings.end(), this);
    size_t index = std::distance(siblings.begin(), it);
    swapRemove(siblings, index);
  }

  _parent = parent;

  if (_parent)
  {
    _parent->_children.emplace_back(this);
  }

  markDirty();
}

void Transform::setPosition(const glm::vec3& position)
{
  if (_position == position)
  {
    return;
  }

  _position = position;
  markDirty();
}

void Transform::setRotation(const glm::quat& rotation)
{
  if (_rotation == rotation)
  {
    return;
  }

  _rotation = rotation;
  markDirty();
}

void Transform::setScale(const glm::vec3& scale)
{
  if (_scale == scale)
  {
    return;
  }

  _scale = scale;
  markDirty();
}

void Transform::setOrigin(const glm::vec3& origin)
{
  if (_origin == origin)
  {
    return;
  }

  _origin = origin;
  markDirty();
}

void Transform::markDirty()
{
  _dirty = true;

  for (auto child : _children)
  {
    child->markDirty();
  }
}

void Transform::recalculateMatrix()
{
  _matrix = _parent ? _parent->matrix() : glm::mat4(1.0f);

  _matrix = glm::translate(_matrix, _position);
  _matrix = glm::translate(_matrix, _origin);
  _matrix *= glm::mat4_cast(_rotation);
  _matrix = glm::scale(_matrix, _scale);
  _matrix = glm::translate(_matrix, -_origin);
}
}  // namespace util
