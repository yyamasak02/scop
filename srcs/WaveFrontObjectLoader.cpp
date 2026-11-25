// Copyright [2026] yyamasak
#include "custom/WaveFrontObjectLoader.hpp"

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "custom/Vec.hpp"

using ft_math::Vec3;

// ---------------------------------------------------------------------------
// 入力検証
//  - 例外は呼び出し元(main)でまとめて catch する。ここでは「どの行が悪いか」
//    が分かるメッセージ付きで std::runtime_error を投げる。
//  - 数値は strtof/strtoll で「トークン全体が数値か」まで確認する
//    (std::stoi は "1.5" の ".5" を黙って捨てる、"1abc" も通すため使わない)。
// ---------------------------------------------------------------------------
void WaveFrontObjectLoader::fail(const std::string& path, int lineNo,
                                 const std::string& msg) {
  throw std::runtime_error(path + ":" + std::to_string(lineNo) + ": " + msg);
}

bool WaveFrontObjectLoader::parseFloat(const std::string& s, float* out) {
  if (s.empty()) return false;
  char* end = nullptr;
  float v = std::strtof(s.c_str(), &end);
  if (end != s.c_str() + s.size()) return false;  // 余りがある(NUL混入含む)
  if (!std::isfinite(v)) return false;            // nan / inf / 桁あふれ
  *out = v;
  return true;
}

// 参照番号1つ("3", "-1")を0始まりの添字に変換する。
// OBJは1始まり、負数は「その時点の要素数から数えて」。0や範囲外は不正。
bool WaveFrontObjectLoader::parseRef(const std::string& s, size_t count,
                                     int* out) {
  if (s.empty()) return false;
  errno = 0;
  char* end = nullptr;
  long long v = std::strtoll(s.c_str(), &end, 10);
  if (end != s.c_str() + s.size() || errno == ERANGE) return false;
  long long n = static_cast<long long>(count);
  long long idx;
  if (v > 0) {
    idx = v - 1;
  } else if (v < 0) {
    idx = n + v;
  } else {
    return false;  // 0
  }
  if (idx < 0 || idx >= n) return false;
  *out = static_cast<int>(idx);
  return true;
}

// 面の1要素。受け付けるのは "v" / "v/vt" / "v//vn" / "v/vt/vn" の4形式だけ。
// テクスチャ(vt)・法線(vn)番号は描画には使わないが、範囲まで検証する
// ("1/", "1/x", "1/1/", "1/1/1/1" などは不正な面として拒否)。
bool WaveFrontObjectLoader::parseFaceToken(const std::string& token,
                                           size_t vCount, size_t vtCount,
                                           size_t vnCount, int* vertex) {
  std::vector<std::string> parts;
  size_t start = 0;
  while (true) {
    const size_t slash = token.find('/', start);
    parts.push_back(token.substr(start, slash - start));
    if (slash == std::string::npos) break;
    start = slash + 1;
  }
  if (parts.size() > 3) return false;

  int unused;
  if (!parseRef(parts[0], vCount, vertex)) return false;
  if (parts.size() == 2 && !parseRef(parts[1], vtCount, &unused))
    return false;  // "v/vt"
  if (parts.size() == 3) {
    // "v//vn" の vt は空でよい。"v/vt/vn" の vt は有効な番号であること
    if (!parts[1].empty() && !parseRef(parts[1], vtCount, &unused))
      return false;
    if (!parseRef(parts[2], vnCount, &unused)) return false;
  }
  return true;
}

// 面の法線の主軸(0=x, 1=y, 2=z)。UV をどの平面に射影するかに使う。
// 最初の3頂点の外積を法線とみなす(面積0なら z)。
int WaveFrontObjectLoader::faceAxis(const std::vector<Vec3>& pos,
                                    const std::vector<int>& face) {
  const Vec3 n =
      ft_math::cross(pos[face[1]] - pos[face[0]], pos[face[2]] - pos[face[0]]);
  const float ax = std::fabs(n.x), ay = std::fabs(n.y), az = std::fabs(n.z);
  if (ax > ay && ax > az) return 0;
  if (ay > az) return 1;
  return 2;
}

// 面ごとの色。隣り合う面が別の灰色になるよう、黄金比でばらけさせる(0.2〜0.75)。
float WaveFrontObjectLoader::faceGrey(size_t faceIndex) {
  double t = static_cast<double>(faceIndex) * 0.6180339887498949;
  t -= std::floor(t);
  return static_cast<float>(0.2 + 0.55 * t);
}

ObjData WaveFrontObjectLoader::load(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open())
    throw std::runtime_error("Cannot open OBJ file: " + path);

  std::vector<Vec3> positions;
  size_t texCoordCount = 0;  // vt の数(面の vt 番号の範囲チェック用)
  size_t normalCount = 0;  // vn の数(面の vn 番号の範囲チェック用)
  std::vector<unsigned char> axes;  // 頂点ごとのUV射影軸
  ObjData data;

  std::string line;
  int lineNo = 0;
  size_t faceCount = 0;
  while (std::getline(file, line)) {
    ++lineNo;
    std::istringstream ss(line);
    std::string token;
    if (!(ss >> token)) continue;  // 空行

    if (token == "v") {
      std::string sx, sy, sz;
      float x, y, z;
      if (!(ss >> sx >> sy >> sz) || !parseFloat(sx, &x) ||
          !parseFloat(sy, &y) || !parseFloat(sz, &z))
        fail(path, lineNo, "invalid vertex (need 3 finite numbers)");
      positions.push_back(Vec3(x, y, z));  // 4つ目以降(w・色)は無視
    } else if (token == "vt") {
      ++texCoordCount;
    } else if (token == "vn") {
      ++normalCount;
    } else if (token == "f") {
      std::vector<int> face;
      std::string item;
      while (ss >> item) {
        int i;
        if (!parseFaceToken(item, positions.size(), texCoordCount, normalCount,
                            &i))
          fail(path, lineNo, "invalid face element: " + item);
        face.push_back(i);
      }
      if (face.size() < 3) fail(path, lineNo, "face needs at least 3 vertices");

      const int axis = faceAxis(positions, face);
      const float grey = faceGrey(faceCount++);
      // 三角形でない面は Fan 分割: (v0, v1, v2), (v0, v2, v3), ...
      // 凸多角形なら正しく分割できる(凹多角形はボーナスなので対象外)
      for (size_t k = 1; k + 1 < face.size(); ++k) {
        for (int vi : {face[0], face[k], face[k + 1]}) {
          data.vertices.push_back(positions[vi].x);
          data.vertices.push_back(positions[vi].y);
          data.vertices.push_back(positions[vi].z);
          data.vertices.push_back(grey);
          data.vertices.push_back(grey);
          data.vertices.push_back(grey);
          data.vertices.push_back(0.0f);  // u (後で計算)
          data.vertices.push_back(0.0f);  // v
          axes.push_back(static_cast<unsigned char>(axis));
        }
      }
    }
    // それ以外(o, g, s, usemtl, mtllib, l, p, #...)は無視
  }
  if (file.bad()) throw std::runtime_error("Read error: " + path);
  if (data.vertices.empty())
    throw std::runtime_error(path + ": no faces found (nothing to draw)");

  const size_t count = data.vertices.size() / kVertexStride;
  data.vertexCount = static_cast<int>(count);
  auto at = [&](size_t v, int c) -> float& {
    return data.vertices[v * kVertexStride + static_cast<size_t>(c)];
  };

  // 中心 = バウンディングボックスの中心。
  // 頂点の平均だと、頂点が密な側に中心が引っ張られ、回転軸がずれる。
  double lo[3] = {1e300, 1e300, 1e300};
  double hi[3] = {-1e300, -1e300, -1e300};
  for (size_t v = 0; v < count; ++v) {
    for (int c = 0; c < 3; ++c) {
      lo[c] = std::min(lo[c], static_cast<double>(at(v, c)));
      hi[c] = std::max(hi[c], static_cast<double>(at(v, c)));
    }
  }
  // 最大の辺が2になるよう拡大縮小(どんな大きさのモデルでも画面に収める)
  double extent = std::max({hi[0] - lo[0], hi[1] - lo[1], hi[2] - lo[2]});
  double scale = extent > 0.0 ? 2.0 / extent : 1.0;

  for (size_t v = 0; v < count; ++v) {
    for (int c = 0; c < 3; ++c) {
      at(v, c) = static_cast<float>((at(v, c) - (lo[c] + hi[c]) / 2.0) * scale);
    }
  }

  // UV: 面の法線の主軸に垂直な平面へ射影する(平面マッピング)。
  // 座標は最大辺が[-1,1]なので、(座標+1)/2 で縦横比を保ったまま[0,1]に収まり、
  // 引き伸ばしが起きない。
  for (size_t v = 0; v < count; ++v) {
    float a, b;
    switch (axes[v]) {
      case 0:  // 法線がx → (y,z)
        a = at(v, 1);
        b = at(v, 2);
        break;
      case 1:  // 法線がy → (x,z)
        a = at(v, 0);
        b = at(v, 2);
        break;
      default:  // 法線がz → (x,y)
        a = at(v, 0);
        b = at(v, 1);
        break;
    }
    at(v, 6) = a * 0.5f + 0.5f;
    at(v, 7) = b * 0.5f + 0.5f;
  }
  return data;
}
