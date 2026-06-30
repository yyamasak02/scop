// Copyright [2026] yyamasak
#pragma once

namespace ft_math {
struct Vec3 {
  float x;
  float y;
  float z;

  Vec3();
  explicit Vec3(const float v);
  Vec3(const float x, const float y, const float z);
  float length() const;
  Vec3 operator*(const float scalar) const;
  Vec3 operator+(const Vec3& target) const;
  Vec3 operator-(const Vec3& target) const;
  Vec3 operator-() const;
  Vec3& operator+=(const Vec3& target);
  Vec3& operator-=(const Vec3& target);
};
/**
 * 正規化
 */
Vec3 normalize(const Vec3& vec);
/**
 * 外積
 */
Vec3 cross(const Vec3& vecA, const Vec3& vecB);
/**
 * 内積
 */
float dot(const Vec3& vecA, const Vec3& vecB);
/**
 * ラジアン
 */
float radians(const float angle);
}  // namespace ft_math
