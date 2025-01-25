#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace graphics
{
class Camera
{
 public:
  Camera();
  ~Camera() = default;

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

  float fov() const
  {
    return _fov;
  }
  void setFov(float fov);

  float aspect() const
  {
    return _aspect;
  }
  void setAspect(float aspect);

  float near() const
  {
    return _near;
  }
  void setNear(float near);

  float far() const
  {
    return _far;
  }
  void setFar(float far);

  const glm::mat4& view() const
  {
    return _view;
  }

  const glm::mat4& projection() const
  {
    return _projection;
  }

  const glm::mat4& viewProjection() const
  {
    return _viewProjection;
  }

 private:
  void recalculateView();
  void recalculateProjection();

 private:
  glm::vec3 _position{0.0f, 0.0f, -5.0f};
  glm::quat _rotation{1.0f, 0.0f, 0.0f, 0.0f};

  float _fov = glm::radians(45.0f);
  float _aspect = 16.0f / 9.0f;
  float _near = 0.1f;
  float _far = 1000.0f;

  glm::mat4 _view{1.0f};
  glm::mat4 _projection{1.0f};
  glm::mat4 _viewProjection{1.0f};
};
}  // namespace graphics
