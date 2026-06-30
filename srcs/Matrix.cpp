// Copyright [2026] yyamasak

#include <cmath>

#include "custom/Matrix.hpp"

namespace ft_math {
Mat4::Mat4() {
  for (int i = 0; i < 16; ++i) {
    this->m[i] = 0.0f;
  }
}

Mat4::Mat4(const float v) : Mat4() {
  this->m[0] = v;
  this->m[5] = v;
  this->m[10] = v;
  this->m[15] = v;
}

float* Mat4::operator[](int col) { return &this->m[col * 4]; }

const float* Mat4::operator[](int col) const { return &this->m[col * 4]; }

Mat4 rotate_y(const float angle) {
  Mat4 m(1.0f);
  m[0][0] = std::cos(angle);
  m[2][0] = std::sin(angle);
  m[0][2] = -std::sin(angle);
  m[2][2] = std::cos(angle);
  return m;
}

Mat4 perspective(const float fovy, const float aspect, const float zNear,
                 const float zFar) {
  Mat4 m(0.0f);

  const float tanHalfFovy = std::tan(fovy / 2.0f);

  m[0][0] = 1.0f / (aspect * tanHalfFovy);
  m[1][1] = 1.0f / tanHalfFovy;
  m[2][2] = -(zFar + zNear) / (zFar - zNear);
  m[2][3] = -1.0f;
  m[3][2] = -(2.0f * zFar * zNear) / (zFar - zNear);
  return m;
}
}  // namespace ft_math
