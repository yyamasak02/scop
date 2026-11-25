// Copyright [2026] yyamasak
#pragma once
#include <string>
#include <vector>

// 1頂点あたりの float 数: x, y, z, r, g, b, u, v
constexpr int kVertexStride = 8;

struct ObjData {
  // Interleaved: x, y, z, r, g, b, u, v per vertex (3頂点で三角形1枚)
  std::vector<float> vertices;
  int vertexCount = 0;
};

// 不正な入力は std::runtime_error を投げる(呼び出し側で catch すること)。
// 戻り値のモデルは、バウンディングボックスの中心が原点、最大辺が長さ2に正規化済み。
ObjData loadObj(const std::string& path);
