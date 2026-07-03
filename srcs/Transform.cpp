// Copyright [2026] yyamasak
#include "custom/Matrix.hpp"
#include "custom/Transform.hpp"
#include "custom/Vec.hpp"

Transform::Transform(const ft_math::Vec3& position, const float rotation)
    : position(position), rotation(rotation) {}

ft_math::Mat4 Transform::getModelMatrix() const {
  return ft_math::translate(position) * ft_math::rotate_y(rotation);
}
