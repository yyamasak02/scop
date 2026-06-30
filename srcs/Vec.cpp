// Copyright [2026] yyamasak
#include <cmath>

#include "custom/Vec.hpp"

namespace ft_math {
Vec3::Vec3() : x(0.0f), y(0.0f), z(0.0f) {}

Vec3::Vec3(const float v) : x(v), y(v), z(v) {}

Vec3::Vec3(const float x, const float y, const float z) : x(x), y(y), z(z) {}

float Vec3::length() const {
  return std::sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
}

Vec3 Vec3::operator*(const float scalar) const {
  return Vec3(this->x * scalar, this->y * scalar, this->z * scalar);
}

Vec3 Vec3::operator+(const Vec3& target) const {
  return Vec3(this->x + target.x, this->y + target.y, this->z + target.z);
}

Vec3 Vec3::operator-(const Vec3& target) const {
  return Vec3(this->x - target.x, this->y - target.y, this->z - target.z);
}

Vec3 Vec3::operator-() const { return Vec3(-this->x, -this->y, -this->z); }

Vec3& Vec3::operator+=(const Vec3& target) {
  this->x += target.x;
  this->y += target.y;
  this->z += target.z;
  return *this;
}

Vec3& Vec3::operator-=(const Vec3& target) {
  this->x -= target.x;
  this->y -= target.y;
  this->z -= target.z;
  return *this;
}

Vec3 normalize(const Vec3& vec) {
  float len = vec.length();
  return Vec3(vec.x / len, vec.y / len, vec.z / len);
}

Vec3 cross(const Vec3& vecA, const Vec3& vecB) {
  float x = vecA.y * vecB.z - vecA.z * vecB.y;
  float y = vecA.z * vecB.x - vecA.x * vecB.z;
  float z = vecA.x * vecB.y - vecA.y * vecB.x;
  return Vec3(x, y, z);
}

float dot(const Vec3& vecA, const Vec3& vecB) {
  return vecA.x * vecB.x + vecA.y * vecB.y + vecA.z * vecB.z;
}
}  // namespace ft_math