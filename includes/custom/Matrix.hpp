// Copyright [2026] yyamasak
#pragma once

struct Mat4 {
  float m[16];

  Mat4();
  explicit Mat4(const float v);
  float* operator[](int col);
  const float* operator[](int col) const;
};

Mat4 rotate_y(const float angle);
