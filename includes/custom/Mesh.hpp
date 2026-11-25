// Copyright [2026] yyamasak
#pragma once
#include <string>
#include <vector>

#include "custom/WaveFrontObjectLoader.hpp"

class Mesh {
 private:
  unsigned int VAO;
  unsigned int VBO;
  int vertexCount;

 public:
  Mesh();
  ~Mesh();
  Mesh(const Mesh&) = delete;  // VAO/VBOを所有するのでコピー禁止
  Mesh& operator=(const Mesh&) = delete;
  void upload(ObjData& objData);
  void draw();
};
