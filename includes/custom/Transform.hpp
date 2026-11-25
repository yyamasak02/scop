// Copyright [2026] yyamasak
#pragma once

#include "custom/Matrix.hpp"
#include "custom/Vec.hpp"

struct Transform {
  ft_math::Vec3 position;
  float rotation;          // Y軸まわりの角度(自動回転 + ユーザー操作)
  float rotationX = 0.0f;  // ユーザー操作によるX軸まわりの角度
  float rotationZ = 0.0f;  // ユーザー操作によるZ軸まわりの角度
  float movementSpeed;
  Transform(const ft_math::Vec3& position, const float rotation,
            const float movementSpeed);

  ft_math::Mat4 getModelMatrix() const;
  void move(const ft_math::Vec3& direction, const float deltaTime);
};
