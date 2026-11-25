// Copyright [2026] yyamasak
#pragma once

#include <string>
class Texture {
 private:
  unsigned int ID = 0;  // 読み込み失敗時も glDeleteTextures に未初期化値を渡さない

 public:
  explicit Texture(const std::string& imgPath);
  ~Texture();
  Texture(const Texture&) = delete;  // コピーすると二重解放になる
  Texture& operator=(const Texture&) = delete;
  void bind(const int& no);
};
