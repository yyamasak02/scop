// Copyright [2026] yyamasak
#pragma once

#include <string>
class Texture {
 private:
  unsigned int ID;

 public:
  explicit Texture(const std::string& imgPath);
  ~Texture();
  void bind(const int& no);
};
