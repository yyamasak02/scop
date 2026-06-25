// Copyright [2026] yyamasak
#pragma once
#include <string>
#include <vector>

struct ObjData {
  // Interleaved: x, y, z, r, g, b per vertex
  std::vector<float> vertices;
  int vertexCount;
};

ObjData loadObj(const std::string& path);
