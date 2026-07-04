// Copyright [2026] yyamasak
#pragma once

#include "custom/Matrix.hpp"
#include "custom/Vec.hpp"

struct Transform {
  ft_math::Vec3 position;
  float rotation;
  float movementSpeed;
  Transform(const ft_math::Vec3& position, const float rotation,
            const float movementSpeed);

  ft_math::Mat4 getModelMatrix() const;
  void move(const ft_math::Vec3& direction, const float deltaTime);
};
