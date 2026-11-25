// Copyright [2026] yyamasak
#pragma once

#include <string>
class Texture {
 private:
  unsigned int ID = 0;

 public:
  explicit Texture(const std::string& imgPath);
  ~Texture();
  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;
  void bind(const int& no);
};
