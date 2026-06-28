// Copyright [2026] yyamasak
// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <iostream>
#include <string>

#include "custom/Texture.hpp"
// clang-format off
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
// clang-format on

Texture::Texture(const std::string& imgPath) {
  glGenTextures(1, &this->ID);
  glBindTexture(GL_TEXTURE_2D, this->ID);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  int texWidth, texHeight, nChannels;
  stbi_set_flip_vertically_on_load(true);
  unsigned char* imgData =
      stbi_load(imgPath.c_str(), &texWidth, &texHeight, &nChannels, 0);
  if (imgData) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, imgData);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cout << "Failed to load texture" << std::endl;
  }
  stbi_image_free(imgData);
}

Texture::~Texture() { glDeleteTextures(1, &this->ID); }

void Texture::bind(const int& no) {
  glActiveTexture(GL_TEXTURE0 + no);
  glBindTexture(GL_TEXTURE_2D, this->ID);
}
