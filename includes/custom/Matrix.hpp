// Copyright [2026] yyamasak
#pragma once

namespace ft_math {
struct Mat4 {
  float m[16];

  Mat4();
  explicit Mat4(const float v);
  float* operator[](int col);
  const float* operator[](int col) const;
};

Mat4 rotate_y(const float angle);
/**
 * 透視投影行列を作成
 */
Mat4 perspective(const float fovy, const float aspect, const float zNear,
                 const float zFar);
}  // namespace ft_math
