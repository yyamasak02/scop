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
  void upload(ObjData& objData);
  void draw();
};
