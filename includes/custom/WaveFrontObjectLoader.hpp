// Copyright [2026] yyamasak
#pragma once
#include <cstddef>
#include <string>
#include <vector>

#include "custom/Vec.hpp"

// 1頂点あたりの float 数: x, y, z, r, g, b, u, v
constexpr int kVertexStride = 8;

struct ObjData {
  // Interleaved: x, y, z, r, g, b, u, v per vertex (3頂点で三角形1枚)
  std::vector<float> vertices;
  int vertexCount = 0;
};

class WaveFrontObjectLoader {
 public:
  static ObjData load(const std::string& path);

 private:
  WaveFrontObjectLoader() = delete;

  static void fail(const std::string& path, int lineNo, const std::string& msg);
  static bool parseFloat(const std::string& s, float* out);
  static bool parseRef(const std::string& s, size_t count, int* out);
  static bool parseFaceToken(const std::string& token, size_t vCount,
                             size_t vtCount, size_t vnCount, int* vertex);

  static int faceAxis(const std::vector<ft_math::Vec3>& pos,
                      const std::vector<int>& face);
  static float faceGrey(size_t faceIndex);
};
