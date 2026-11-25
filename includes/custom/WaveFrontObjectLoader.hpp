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

  struct P2 {
    double x;
    double y;
  };

  struct Tri {
    int a;
    int b;
    int c;
  };

  enum class FaceResult {
    Ok,
    Degenerate,
    SelfIntersecting,
    NoEar,
  };

  static void fail(const std::string& path, int lineNo, const std::string& msg);
  static bool parseFloat(const std::string& s, float* out);
  static bool parseRef(const std::string& s, size_t count, int* out);
  static bool parseFaceToken(const std::string& token, size_t vCount,
                             size_t vtCount, size_t vnCount, int* vertex);

  static double cross2(const P2& a, const P2& b, const P2& c);
  static bool same(const P2& a, const P2& b);
  static bool same3(const ft_math::Vec3& a, const ft_math::Vec3& b);
  static void newell(const std::vector<ft_math::Vec3>& pos,
                     const std::vector<int>& poly, double nrm[3]);
  static int dominantAxis(const double nrm[3]);
  static bool allCollinear(const std::vector<ft_math::Vec3>& pos,
                           const std::vector<int>& poly);
  static bool onSegment(const P2& a, const P2& b, const P2& c, double tol);
  static int orient(const P2& a, const P2& b, const P2& c, double eps);
  static bool segmentsTouch(const P2& a, const P2& b, const P2& c, const P2& d,
                            double eps, double tol);

  static bool isSimple(const std::vector<P2>& pt, double eps, double tol);
  static bool earClip(const std::vector<P2>& pt, const std::vector<int>& poly,
                      double sign, double eps, std::vector<Tri>* out);
  static FaceResult triangulate(const std::vector<ft_math::Vec3>& pos,
                                const std::vector<int>& face,
                                std::vector<Tri>* out, int* axisOut);

  static float faceGrey(size_t faceIndex);
};
