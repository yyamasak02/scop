// Copyright [2026] yyamasak
#include <glad/glad.h>

#include <cmath>

#include "custom/Camera.hpp"
#include "custom/Matrix.hpp"
#include "custom/Vec.hpp"

Camera::Camera(ft_math::Vec3 position, ft_math::Vec3 up, float yaw, float pitch)
    : front(ft_math::Vec3(0.0f, 0.0f, -1.0f)),
      movementSpeed(SPEED),
      mouseSensitivity(SENSITIVITY),
      zoom(ZOOM) {
  this->position = position;
  this->worldUp = up;
  this->yaw = yaw;
  this->pitch = pitch;
  updateCameraVectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY,
               float upZ, float yaw, float pitch)
    : front(ft_math::Vec3(0.0f, 0.0f, -1.0f)),
      movementSpeed(SPEED),
      mouseSensitivity(SENSITIVITY),
      zoom(ZOOM) {
  this->position = ft_math::Vec3(posX, posY, posZ);
  this->worldUp = ft_math::Vec3(upX, upY, upZ);
  this->yaw = yaw;
  this->pitch = pitch;
  updateCameraVectors();
}

ft_math::Mat4 Camera::GetViewMatrix() {
  ft_math::Vec3 zaxis = ft_math::normalize(-this->front);
  ft_math::Vec3 xaxis =
      ft_math::normalize(ft_math::cross(this->worldUp, zaxis));
  ft_math::Vec3 yaxis = ft_math::cross(zaxis, xaxis);
  ft_math::Mat4 m;
  m[0][0] = xaxis.x;
  m[0][1] = yaxis.x;
  m[0][2] = zaxis.x;
  m[0][3] = 0.0f;
  m[1][0] = xaxis.y;
  m[1][1] = yaxis.y;
  m[1][2] = zaxis.y;
  m[1][3] = 0.0f;
  m[2][0] = xaxis.z;
  m[2][1] = yaxis.z;
  m[2][2] = zaxis.z;
  m[2][3] = 0.0f;
  m[3][0] = -ft_math::dot(xaxis, position);
  m[3][1] = -ft_math::dot(yaxis, position);
  m[3][2] = -ft_math::dot(zaxis, position);
  m[3][3] = 1.0f;
  return m;
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
  float velocity = this->movementSpeed * deltaTime;
  if (direction == FORWARD) this->position += this->front * velocity;
  if (direction == BACKWARD) this->position -= this->front * velocity;
  if (direction == LEFT) this->position -= this->right * velocity;
  if (direction == RIGHT) this->position += this->right * velocity;
  if (direction == UP) this->position += this->up * velocity;
  if (direction == DOWN) this->position -= this->up * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset,
                                  GLboolean constrainPitch) {
  xoffset *= this->mouseSensitivity;
  yoffset *= this->mouseSensitivity;

  this->yaw += xoffset;
  this->pitch += yoffset;

  // make sure that when pitch is out of bounds, screen doesn't get flipped
  if (constrainPitch) {
    if (this->pitch > 89.0f) this->pitch = 89.0f;
    if (this->pitch < -89.0f) this->pitch = -89.0f;
  }

  // update Front, Right and Up Vectors using the updated Euler angles
  updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
  this->zoom -= static_cast<float>(yoffset);
  if (this->zoom < 1.0f) this->zoom = 1.0f;
  if (this->zoom > 45.0f) this->zoom = 45.0f;
}

void Camera::updateCameraVectors() {
  // calculate the new Front vector
  float x =
      cos(ft_math::radians(this->yaw)) * cos(ft_math::radians(this->pitch));
  float y = sin(ft_math::radians(this->pitch));
  float z =
      sin(ft_math::radians(this->yaw)) * cos(ft_math::radians(this->pitch));
  ft_math::Vec3 front(x, y, z);
  this->front = ft_math::normalize(front);
  // also re-calculate the Right and Up vector
  this->right = ft_math::normalize(cross(front, this->worldUp));
  // normalize the vectors, because their length
  // gets closer to 0 the more you look up or down
  // which results in slower movement.
  this->up = ft_math::normalize(ft_math::cross(this->right, this->front));
}
