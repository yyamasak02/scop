// Copyright [2026] yyamasak
// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <iostream>
#include <string>

#include "custom/ImageLoader.hpp"
#include "custom/Texture.hpp"

Texture::Texture(const std::string& imgPath) {
  ft_image::ImageData imgData = ft_image::loadImage(imgPath);
  if (imgData.data.empty()) {
    std::cout << "Failed to load texture: " << imgPath << std::endl;
    return;
  }
  glGenTextures(1, &this->ID);
  glBindTexture(GL_TEXTURE_2D, this->ID);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, imgData.internalFormat, imgData.width,
               imgData.height, 0, imgData.format, GL_UNSIGNED_BYTE,
               imgData.data.data());
  glGenerateMipmap(GL_TEXTURE_2D);
}

Texture::~Texture() { glDeleteTextures(1, &this->ID); }

void Texture::bind(const int& no) {
  glActiveTexture(GL_TEXTURE0 + no);
  glBindTexture(GL_TEXTURE_2D, this->ID);
}
