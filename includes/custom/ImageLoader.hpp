// Copyright [2026] yyamasak
#pragma once
// clang-format off
#include <glad/glad.h>
// clang-format on

#include <string>
#include <vector>

namespace ft_image {
struct ImageData {
  int width;
  int height;
  int bpp;
  GLenum format;
  GLenum internalFormat;
  std::vector<unsigned char> data;
};
ImageData loadImage(const std::string& image_path);
}  // namespace ft_image
