// Copyright [2026] yyamasak
#include "custom/Matrix.hpp"
#include "custom/Transform.hpp"
#include "custom/Vec.hpp"

Transform::Transform(const ft_math::Vec3& position, const float rotation,
                     const float movementSpeed)
    : position(position), rotation(rotation), movementSpeed(movementSpeed) {}

ft_math::Mat4 Transform::getModelMatrix() const {
  // 右から順に適用: 物体の中心(原点)でY→Z→X回転し、最後にワールド位置へ移動。
  // 回転を先にするので、移動後も物体は自分の中心軸まわりに回る。
  return ft_math::translate(position) * ft_math::rotateX(rotationX) *
         ft_math::rotateZ(rotationZ) * ft_math::rotateY(rotation);
}

void Transform::move(const ft_math::Vec3& direction, float deltaTime) {
  this->position += direction * this->movementSpeed * deltaTime;
}
