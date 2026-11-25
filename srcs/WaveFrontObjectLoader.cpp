// Copyright [2026] yyamasak
#include <algorithm>
#include <array>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "custom/Vec.hpp"
#include "custom/WaveFrontObjectLoader.hpp"

namespace {

using ft_math::Vec3;

// ---------------------------------------------------------------------------
// 入力検証
//  - 例外は呼び出し元(main)でまとめて catch する。ここでは「どの行が悪いか」
//    が分かるメッセージ付きで std::runtime_error を投げる。
//  - 数値は strtof/strtoll で「トークン全体が数値か」まで確認する
//    (std::stoi は "1.5" の ".5" を黙って捨てる、"1abc" も通すため使わない)。
// ---------------------------------------------------------------------------
[[noreturn]] void fail(const std::string& path, int lineNo,
                       const std::string& msg) {
  throw std::runtime_error(path + ":" + std::to_string(lineNo) + ": " + msg);
}

bool parseFloat(const std::string& s, float* out) {
  if (s.empty()) return false;
  char* end = nullptr;
  float v = std::strtof(s.c_str(), &end);
  if (end != s.c_str() + s.size()) return false;  // 余りがある(NUL混入含む)
  if (!std::isfinite(v)) return false;            // nan / inf / 桁あふれ
  *out = v;
  return true;
}

// "v", "v/vt", "v//vn", "v/vt/vn" の先頭(頂点番号)だけを読む。
// OBJは1始まり、負数は「現在の頂点数から数えて」。0や範囲外は不正。
bool parseIndex(const std::string& token, size_t vertexCount, int* out) {
  std::string head = token.substr(0, token.find('/'));
  if (head.empty()) return false;
  errno = 0;
  char* end = nullptr;
  long long v = std::strtoll(head.c_str(), &end, 10);
  if (end != head.c_str() + head.size() || errno == ERANGE) return false;
  long long n = static_cast<long long>(vertexCount);
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

// ---------------------------------------------------------------------------
// 多角形の三角形分割 (Ear Clipping)
//  Fan分割 (v0を起点に扇状) は凸多角形でしか正しくない。
//  凹多角形では「耳(ear)」= 隣り合う3頂点がつくる三角形のうち、
//   1. 凸(多角形の内側に折れている)で
//   2. 内部に他の頂点を含まない
//  ものを1つずつ切り落とす。n角形からn-2枚の三角形が得られる。
//  3D多角形を2Dで判定するため、Newell法で法線を求め、法線の最大成分の軸を
//  落として2Dに射影する(軸を巡回順に選ぶので、射影後の巻き方向が法線の符号と一致)。
// ---------------------------------------------------------------------------
struct P2 {
  double x;
  double y;
};

double cross2(const P2& a, const P2& b, const P2& c) {
  return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

bool same(const P2& a, const P2& b) { return a.x == b.x && a.y == b.y; }

struct Tri {
  int a;
  int b;
  int c;
};

// 戻り値: 多角形の法線の最大成分の軸(0=x,1=y,2=z)。UVの射影軸にも使う。
int triangulate(const std::vector<Vec3>& pos, const std::vector<int>& poly,
                std::vector<Tri>* out) {
  const size_t n = poly.size();
  double nrm[3] = {0.0, 0.0, 0.0};
  for (size_t i = 0; i < n; ++i) {
    const Vec3& p = pos[poly[i]];
    const Vec3& q = pos[poly[(i + 1) % n]];
    nrm[0] += (static_cast<double>(p.y) - q.y) * (static_cast<double>(p.z) + q.z);
    nrm[1] += (static_cast<double>(p.z) - q.z) * (static_cast<double>(p.x) + q.x);
    nrm[2] += (static_cast<double>(p.x) - q.x) * (static_cast<double>(p.y) + q.y);
  }
  int axis = 0;
  for (int k = 1; k < 3; ++k) {
    if (std::fabs(nrm[k]) > std::fabs(nrm[axis])) axis = k;
  }

  // 面積0(一直線・同一点)の多角形は判定不能。Fanで分割だけしておく。
  if (std::fabs(nrm[axis]) <= 0.0) {
    for (size_t i = 1; i + 1 < n; ++i)
      out->push_back({poly[0], poly[i], poly[i + 1]});
    return 2;
  }

  std::vector<P2> pt(n);
  double minX = 1e300, maxX = -1e300, minY = 1e300, maxY = -1e300;
  for (size_t i = 0; i < n; ++i) {
    const Vec3& v = pos[poly[i]];
    if (axis == 0) {
      pt[i] = {v.y, v.z};
    } else if (axis == 1) {
      pt[i] = {v.z, v.x};
    } else {
      pt[i] = {v.x, v.y};
    }
    minX = std::min(minX, pt[i].x);
    maxX = std::max(maxX, pt[i].x);
    minY = std::min(minY, pt[i].y);
    maxY = std::max(maxY, pt[i].y);
  }
  const double extent = std::max(maxX - minX, maxY - minY);
  const double eps = 1e-12 * extent * extent;
  const double sign = nrm[axis] > 0.0 ? 1.0 : -1.0;  // 反時計回りなら +1

  std::vector<int> idx(n);
  std::iota(idx.begin(), idx.end(), 0);

  while (idx.size() > 3) {
    const size_t m = idx.size();
    bool found = false;
    size_t fallback = 0;
    double fallbackConv = -1e300;

    for (size_t i = 0; i < m && !found; ++i) {
      const int ip = idx[(i + m - 1) % m];
      const int ic = idx[i];
      const int iq = idx[(i + 1) % m];
      const double conv = sign * cross2(pt[ip], pt[ic], pt[iq]);
      if (conv > fallbackConv) {
        fallbackConv = conv;
        fallback = i;
      }
      if (conv < -eps) continue;  // 凹(reflex)の頂点は耳にならない

      bool blocked = false;
      for (size_t j = 0; j < m && !blocked; ++j) {
        const int ij = idx[j];
        if (ij == ip || ij == ic || ij == iq) continue;
        // 角と同じ座標の頂点(閉じ頂点の重複など)は無視する
        if (same(pt[ij], pt[ip]) || same(pt[ij], pt[ic]) ||
            same(pt[ij], pt[iq]))
          continue;
        // 三角形の内側か「辺上」にある頂点があれば、その耳は切れない。
        // 辺上を許すと、凹頂点が斜辺にちょうど乗るL字などで、
        // 凹みをまたぐ三角形ができてしまう。
        const double d1 = sign * cross2(pt[ip], pt[ic], pt[ij]);
        const double d2 = sign * cross2(pt[ic], pt[iq], pt[ij]);
        const double d3 = sign * cross2(pt[iq], pt[ip], pt[ij]);
        if (d1 >= -eps && d2 >= -eps && d3 >= -eps) blocked = true;
      }
      if (blocked) continue;

      out->push_back({poly[ip], poly[ic], poly[iq]});
      idx.erase(idx.begin() + static_cast<long>(i));
      found = true;
    }

    if (!found) {
      // 自己交差などで耳が見つからない不正な多角形。必ず終了するよう、
      // 最も凸な頂点を強制的に切り落とす(無限ループ防止)。
      const int ip = idx[(fallback + m - 1) % m];
      const int ic = idx[fallback];
      const int iq = idx[(fallback + 1) % m];
      out->push_back({poly[ip], poly[ic], poly[iq]});
      idx.erase(idx.begin() + static_cast<long>(fallback));
    }
  }
  out->push_back({poly[idx[0]], poly[idx[1]], poly[idx[2]]});
  return axis;
}

// 面ごとの色。隣り合う面が別の灰色になるよう、黄金比でばらけさせる(0.2〜0.75)。
float faceGrey(size_t faceIndex) {
  double t = static_cast<double>(faceIndex) * 0.6180339887498949;
  t -= std::floor(t);
  return static_cast<float>(0.2 + 0.55 * t);
}

}  // namespace

ObjData loadObj(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open())
    throw std::runtime_error("Cannot open OBJ file: " + path);

  std::vector<Vec3> positions;
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
    } else if (token == "f") {
      std::vector<int> poly;
      std::string idx;
      while (ss >> idx) {
        int i;
        if (!parseIndex(idx, positions.size(), &i))
          fail(path, lineNo, "invalid face index: " + idx);
        poly.push_back(i);
      }
      if (poly.size() < 3) fail(path, lineNo, "face needs at least 3 vertices");

      std::vector<Tri> tris;
      int axis = triangulate(positions, poly, &tris);
      const float grey = faceGrey(faceCount++);
      for (const Tri& t : tris) {
        for (int vi : {t.a, t.b, t.c}) {
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
    // それ以外(vn, vt, o, g, s, usemtl, mtllib, l, p, #...)は無視
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
