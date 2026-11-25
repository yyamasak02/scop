// OBJパーサー単体テスト用ハーネス（ウィンドウ不要）
// 使い方: obj_harness <file.obj>
// 終了コード: 0=読み込み成功(健全) / 1=例外をcatchして拒否 / 2=読み込めたが不正な結果
//             それ以外(シグナル)=クラッシュ
#include <cmath>
#include <cstdio>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>

#include "custom/WaveFrontObjectLoader.hpp"

static bool readExpect(const std::string& path, const std::string& key,
                       double* out) {
  std::ifstream f(path);
  std::string line;
  const std::string tag = "# " + key + " ";
  while (std::getline(f, line)) {
    if (line.rfind(tag, 0) == 0) {
      *out = std::stod(line.substr(tag.size()));
      return true;
    }
  }
  return false;
}

int main(int ac, char** av) {
  if (ac != 2) {
    std::cerr << "usage: obj_harness <file.obj>\n";
    return 64;
  }
  try {
    ObjData d = WaveFrontObjectLoader::load(av[1]);

    if (d.vertexCount <= 0 || d.vertices.empty() || d.vertexCount % 3 != 0) {
      std::printf("BAD vertexCount=%d\n", d.vertexCount);
      return 2;
    }
    // ストライドは実装(色のみ=6, UV付き=8など)に依存しないよう求める
    size_t stride = d.vertices.size() / static_cast<size_t>(d.vertexCount);
    if (stride < 3 || stride * d.vertexCount != d.vertices.size()) {
      std::printf("BAD stride\n");
      return 2;
    }
    for (float v : d.vertices) {
      if (!std::isfinite(v)) {
        std::printf("BAD non-finite value in vertex buffer\n");
        return 2;
      }
    }
    // 正規化の確認: 中心が原点で、すべての座標が[-1,1]、UVが[0,1]に収まること
    if (stride >= 8) {
      for (int v = 0; v < d.vertexCount; ++v) {
        const float* p = &d.vertices[v * stride];
        for (int c = 0; c < 3; ++c) {
          if (std::fabs(p[c]) > 1.0001f) {
            std::printf("BAD position out of [-1,1]: %f\n", p[c]);
            return 2;
          }
        }
        if (p[6] < -0.0001f || p[6] > 1.0001f || p[7] < -0.0001f ||
            p[7] > 1.0001f) {
          std::printf("BAD uv out of [0,1]: %f %f\n", p[6], p[7]);
          return 2;
        }
      }
    }

    double area = 0.0;
    int tris = d.vertexCount / 3;
    for (int t = 0; t < tris; ++t) {
      const float* a = &d.vertices[(t * 3 + 0) * stride];
      const float* b = &d.vertices[(t * 3 + 1) * stride];
      const float* c = &d.vertices[(t * 3 + 2) * stride];
      double ux = b[0] - a[0], uy = b[1] - a[1], uz = b[2] - a[2];
      double vx = c[0] - a[0], vy = c[1] - a[1], vz = c[2] - a[2];
      double cx = uy * vz - uz * vy, cy = uz * vx - ux * vz,
             cz = ux * vy - uy * vx;
      area += 0.5 * std::sqrt(cx * cx + cy * cy + cz * cz);
    }

    double expect;
    if (readExpect(av[1], "EXPECT_TRIS", &expect) &&
        tris != static_cast<int>(expect)) {
      std::printf("BAD tris=%d expected=%d\n", tris, static_cast<int>(expect));
      return 2;
    }
    // ローダーはモデルを「最大辺=2」に拡大縮小するので、元の大きさに戻して比べる
    {
      double lo[3] = {1e300, 1e300, 1e300}, hi[3] = {-1e300, -1e300, -1e300};
      std::ifstream f(av[1]);
      std::string tag;
      while (f >> tag) {
        if (tag == "v") {
          double x[3];
          f >> x[0] >> x[1] >> x[2];
          for (int c = 0; c < 3; ++c) {
            lo[c] = std::fmin(lo[c], x[c]);
            hi[c] = std::fmax(hi[c], x[c]);
          }
        }
      }
      double ext = std::fmax(hi[0] - lo[0],
                             std::fmax(hi[1] - lo[1], hi[2] - lo[2]));
      if (ext > 0) area *= (ext / 2.0) * (ext / 2.0);
    }
    if (readExpect(av[1], "EXPECT_AREA", &expect) &&
        std::fabs(area - expect) > 1e-3 * (expect > 1 ? expect : 1)) {
      std::printf("BAD area=%.4f expected=%.4f\n", area, expect);
      return 2;
    }
    std::printf("LOADED tris=%d area=%.4f\n", tris, area);
    return 0;
  } catch (const std::exception& e) {
    std::printf("REJECTED (%s)\n", e.what());
    return 1;
  } catch (...) {
    std::printf("REJECTED (non-std exception)\n");
    return 1;
  }
}
