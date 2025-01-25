#include "camera.h"

namespace graphics
{
Camera::Camera()
{
  recalculateProjection();
  recalculateView();
}

void Camera::setPosition(const glm::vec3& position)
{
  if (_position == position)
  {
    return;
  }
  _position = position;
  recalculateView();
}

void Camera::setRotation(const glm::quat& rotation)
{
  if (_rotation == rotation)
  {
    return;
  }
  _rotation = rotation;
  recalculateView();
}

void Camera::setFov(float fov)
{
  if (_fov == fov)
  {
    return;
  }
  _fov = fov;
  recalculateProjection();
}

void Camera::setAspect(float aspect)
{
  if (_aspect == aspect)
  {
    return;
  }
  _aspect = aspect;
  recalculateProjection();
}

void Camera::setNear(float near)
{
  if (_near == near)
  {
    return;
  }
  _near = near;
  recalculateProjection();
}

void Camera::setFar(float far)
{
  if (_far == far)
  {
    return;
  }
  _far = far;
  recalculateProjection();
}

void Camera::recalculateView()
{
  _view =
    glm::translate(glm::mat4(1.0f), _position) * glm::mat4_cast(_rotation);
  _viewProjection = _projection * glm::inverse(_view);
}

void Camera::recalculateProjection()
{
  _projection = glm::perspective(_fov, _aspect, _near, _far);
  _viewProjection = _projection * glm::inverse(_view);
}
}  // namespace graphics
