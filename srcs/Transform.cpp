// Copyright [2026] yyamasak
#include "custom/Matrix.hpp"
#include "custom/Transform.hpp"
#include "custom/Vec.hpp"

Transform::Transform(const ft_math::Vec3& position, const float rotation,
                     const float movementSpeed)
    : position(position), rotation(rotation), movementSpeed(movementSpeed) {}

ft_math::Mat4 Transform::getModelMatrix() const {
  return ft_math::translate(position) * ft_math::rotate_y(rotation);
}

void Transform::move(const ft_math::Vec3& direction, float deltaTime) {
  this->position += direction * this->movementSpeed * deltaTime;
}
