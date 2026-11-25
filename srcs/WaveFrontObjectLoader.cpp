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

// ---------------------------------------------------------------------------
// 多角形の三角形分割
//  Fan分割 (v0を起点に扇状) は凸多角形でしか正しくないので、Ear Clipping
//  を使う。 「耳(ear)」= 隣り合う3頂点がつくる三角形のうち、
//   1. 凸(多角形の内側に折れている)で
//   2. 内部・辺上に他の頂点を含まない
//  もの。単純多角形(自己交差しない多角形)には必ず耳があり、
//  1つずつ切り落とすと n角形から n-2 枚の三角形が得られる。
//
//  3D多角形を2Dで判定するため、Newell法で法線を求め、法線の最大成分の軸を
//  落として2Dに射影する(軸を巡回順に選ぶので、射影後の巻き方向が法線の符号と一致)。
//
//  不正な多角形から形をでっち上げないよう、次の方針をとる:
//   - 三角形(3頂点)は入力そのものなので、面積0でもそのまま使う
//   - 4頂点以上で面積0(一直線・同一点)の面は、描くものがないので出力しない
//   - 自己交差・接触する面は拒否する(例外)
// ---------------------------------------------------------------------------
double WaveFrontObjectLoader::cross2(const P2& a, const P2& b, const P2& c) {
  return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

bool WaveFrontObjectLoader::same(const P2& a, const P2& b) {
  return a.x == b.x && a.y == b.y;
}

bool WaveFrontObjectLoader::same3(const Vec3& a, const Vec3& b) {
  return a.x == b.x && a.y == b.y && a.z == b.z;
}

// Newell法による多角形の法線(長さ = 面積の2倍)
void WaveFrontObjectLoader::newell(const std::vector<Vec3>& pos,
                                   const std::vector<int>& poly,
                                   double nrm[3]) {
  nrm[0] = nrm[1] = nrm[2] = 0.0;
  const size_t n = poly.size();
  for (size_t i = 0; i < n; ++i) {
    const Vec3& p = pos[poly[i]];
    const Vec3& q = pos[poly[(i + 1) % n]];
    nrm[0] +=
        (static_cast<double>(p.y) - q.y) * (static_cast<double>(p.z) + q.z);
    nrm[1] +=
        (static_cast<double>(p.z) - q.z) * (static_cast<double>(p.x) + q.x);
    nrm[2] +=
        (static_cast<double>(p.x) - q.x) * (static_cast<double>(p.y) + q.y);
  }
}

int WaveFrontObjectLoader::dominantAxis(const double nrm[3]) {
  int axis = 2;  // 法線が求まらない(面積0)ときの既定
  double best = 0.0;
  for (int k = 0; k < 3; ++k) {
    if (std::fabs(nrm[k]) > best) {
      best = std::fabs(nrm[k]);
      axis = k;
    }
  }
  return axis;
}

// すべての頂点が一直線上(または同一点)にあるか
bool WaveFrontObjectLoader::allCollinear(const std::vector<Vec3>& pos,
                                         const std::vector<int>& poly) {
  const Vec3& p0 = pos[poly[0]];
  double lx = 0, ly = 0, lz = 0, best = 0;
  for (int i : poly) {
    const double dx = pos[i].x - p0.x, dy = pos[i].y - p0.y,
                 dz = pos[i].z - p0.z;
    const double d = dx * dx + dy * dy + dz * dz;
    if (d > best) {
      best = d;
      lx = dx;
      ly = dy;
      lz = dz;
    }
  }
  if (best == 0.0) return true;  // 全部同じ点
  for (int i : poly) {
    const double dx = pos[i].x - p0.x, dy = pos[i].y - p0.y,
                 dz = pos[i].z - p0.z;
    const double cx = ly * dz - lz * dy, cy = lz * dx - lx * dz,
                 cz = lx * dy - ly * dx;
    if (cx * cx + cy * cy + cz * cz > 1e-18 * best * best) return false;
  }
  return true;
}

// c が線分ab上にあるか(c は ab と同一直線上である前提)
bool WaveFrontObjectLoader::onSegment(const P2& a, const P2& b, const P2& c,
                                      double tol) {
  return std::min(a.x, b.x) - tol <= c.x && c.x <= std::max(a.x, b.x) + tol &&
         std::min(a.y, b.y) - tol <= c.y && c.y <= std::max(a.y, b.y) + tol;
}

int WaveFrontObjectLoader::orient(const P2& a, const P2& b, const P2& c,
                                  double eps) {
  const double v = cross2(a, b, c);
  return v > eps ? 1 : (v < -eps ? -1 : 0);
}

// 線分abとcdが交わるか接するか(端点の接触・同一直線上の重なりも含む)
bool WaveFrontObjectLoader::segmentsTouch(const P2& a, const P2& b, const P2& c,
                                          const P2& d, double eps, double tol) {
  const int o1 = orient(a, b, c, eps), o2 = orient(a, b, d, eps);
  const int o3 = orient(c, d, a, eps), o4 = orient(c, d, b, eps);
  if (o1 * o2 < 0 && o3 * o4 < 0) return true;
  if (o1 == 0 && onSegment(a, b, c, tol)) return true;
  if (o2 == 0 && onSegment(a, b, d, tol)) return true;
  if (o3 == 0 && onSegment(c, d, a, tol)) return true;
  if (o4 == 0 && onSegment(c, d, b, tol)) return true;
  return false;
}

// 単純多角形か(自己交差・接触・折り返しがないか)。
// 全辺のペアを比べると O(n^2) で大きな面が遅くなるので、格子に辺を登録し、
// 同じマスに入った辺どうしだけを比べる。
bool WaveFrontObjectLoader::isSimple(const std::vector<P2>& pt, double eps,
                                     double tol) {
  const size_t n = pt.size();

  // 隣り合う辺の折り返し(a→b→aのようなスパイク)
  for (size_t i = 0; i < n; ++i) {
    const P2& a = pt[(i + n - 1) % n];
    const P2& b = pt[i];
    const P2& c = pt[(i + 1) % n];
    if (std::fabs(cross2(a, b, c)) <= eps) {
      const double dot = (a.x - b.x) * (c.x - b.x) + (a.y - b.y) * (c.y - b.y);
      if (dot > 0) return false;
    }
  }

  double minX = pt[0].x, maxX = pt[0].x, minY = pt[0].y, maxY = pt[0].y;
  for (const P2& p : pt) {
    minX = std::min(minX, p.x);
    maxX = std::max(maxX, p.x);
    minY = std::min(minY, p.y);
    maxY = std::max(maxY, p.y);
  }
  const int g =
      std::max(1, static_cast<int>(std::sqrt(static_cast<double>(n))));
  const double cw = std::max(maxX - minX, 1e-300) / g;
  const double ch = std::max(maxY - minY, 1e-300) / g;
  auto cellX = [&](double x) {
    return std::clamp(static_cast<int>((x - minX) / cw), 0, g - 1);
  };
  auto cellY = [&](double y) {
    return std::clamp(static_cast<int>((y - minY) / ch), 0, g - 1);
  };

  std::vector<std::vector<int>> cells(static_cast<size_t>(g) * g);
  for (size_t e = 0; e < n; ++e) {
    const P2& a = pt[e];
    const P2& b = pt[(e + 1) % n];
    const int x0 = cellX(std::min(a.x, b.x) - tol);
    const int x1 = cellX(std::max(a.x, b.x) + tol);
    const int y0 = cellY(std::min(a.y, b.y) - tol);
    const int y1 = cellY(std::max(a.y, b.y) + tol);
    for (int cy = y0; cy <= y1; ++cy)
      for (int cx = x0; cx <= x1; ++cx)
        cells[static_cast<size_t>(cy) * g + cx].push_back(static_cast<int>(e));
  }

  for (const std::vector<int>& cell : cells) {
    for (size_t i = 0; i < cell.size(); ++i) {
      for (size_t j = i + 1; j < cell.size(); ++j) {
        const size_t e = static_cast<size_t>(cell[i]);
        const size_t f = static_cast<size_t>(cell[j]);
        if ((e + 1) % n == f || (f + 1) % n == e) continue;  // 隣接辺
        if (segmentsTouch(pt[e], pt[(e + 1) % n], pt[f], pt[(f + 1) % n], eps,
                          tol))
          return false;
      }
    }
  }
  return true;
}

// Ear Clipping 本体。
//  - 頂点は双方向連結リスト(prev/next)で持ち、削除を O(1) にする
//  - 耳の内側に入りうるのは凹頂点(と180°の頂点)だけなので、それだけと比べる
//    (凸多角形ではほぼ O(n) で終わる)
//  - 耳を切ったら先頭からやり直さず、その隣から探索を続ける
bool WaveFrontObjectLoader::earClip(const std::vector<P2>& pt,
                                    const std::vector<int>& poly, double sign,
                                    double eps, std::vector<Tri>* out) {
  const int n = static_cast<int>(pt.size());
  std::vector<int> prv(n), nxt(n);
  for (int i = 0; i < n; ++i) {
    prv[i] = (i + n - 1) % n;
    nxt[i] = (i + 1) % n;
  }
  auto convexity = [&](int i) {
    return sign * cross2(pt[prv[i]], pt[i], pt[nxt[i]]);
  };

  std::vector<char> alive(n, 1), blocker(n, 0);
  std::vector<int> blockers;  // 凹頂点・180°の頂点(耳をふさぎうる頂点)
  for (int i = 0; i < n; ++i) {
    if (convexity(i) <= eps) {
      blocker[i] = 1;
      blockers.push_back(i);
    }
  }
  size_t stale = 0;  // blockers 内の、もう対象でない要素の数

  auto isEar = [&](int c) {
    if (convexity(c) < -eps) return false;  // 凹頂点は耳にならない
    const int p = prv[c], q = nxt[c];
    for (int j : blockers) {
      if (!alive[j] || !blocker[j] || j == p || j == c || j == q) continue;
      // 角と同じ座標の頂点は無視する
      if (same(pt[j], pt[p]) || same(pt[j], pt[c]) || same(pt[j], pt[q]))
        continue;
      // 内側か「辺上」にあれば、その耳は切れない
      // (辺上を許すと、凹頂点が斜辺にちょうど乗るL字などで凹みをまたぐ)
      if (sign * cross2(pt[p], pt[c], pt[j]) >= -eps &&
          sign * cross2(pt[c], pt[q], pt[j]) >= -eps &&
          sign * cross2(pt[q], pt[p], pt[j]) >= -eps)
        return false;
    }
    return true;
  };

  int remaining = n;
  int cur = 0;
  int fails = 0;
  while (remaining > 3) {
    if (!isEar(cur)) {
      cur = nxt[cur];
      if (++fails > remaining) return false;  // 一周しても耳がない
      continue;
    }
    const int p = prv[cur], q = nxt[cur];
    out->push_back({poly[p], poly[cur], poly[q]});
    alive[cur] = 0;
    if (blocker[cur]) ++stale;
    nxt[p] = q;
    prv[q] = p;
    --remaining;

    // 両隣の凹凸が変わりうるので更新
    for (int v : {p, q}) {
      const bool isBlocker = convexity(v) <= eps;
      if (isBlocker && !blocker[v]) blockers.push_back(v);
      if (!isBlocker && blocker[v]) ++stale;
      blocker[v] = isBlocker ? 1 : 0;
    }
    if (stale * 2 > blockers.size()) {  // 不要な要素がたまったら詰める
      std::vector<int> kept;
      for (int j : blockers)
        if (alive[j] && blocker[j]) kept.push_back(j);
      blockers.swap(kept);
      stale = 0;
    }
    cur = p;
    fails = 0;
  }
  out->push_back({poly[prv[cur]], poly[cur], poly[nxt[cur]]});
  return true;
}

WaveFrontObjectLoader::FaceResult WaveFrontObjectLoader::triangulate(
    const std::vector<Vec3>& pos, const std::vector<int>& face,
    std::vector<Tri>* out, int* axisOut) {
  double nrm[3];

  // 三角形は入力そのもの。分割も補正もせずに使う
  if (face.size() == 3) {
    newell(pos, face, nrm);
    *axisOut = dominantAxis(nrm);
    out->push_back({face[0], face[1], face[2]});
    return FaceResult::Ok;
  }

  // 連続する同じ座標の頂点をまとめる(閉じるために先頭を末尾にも書く例など)。
  // 形は変わらない正規化なので、でっち上げにはならない。
  std::vector<int> poly;
  for (int i : face) {
    if (poly.empty() || !same3(pos[poly.back()], pos[i])) poly.push_back(i);
  }
  while (poly.size() > 1 && same3(pos[poly.front()], pos[poly.back()]))
    poly.pop_back();
  if (poly.size() < 3) return FaceResult::Degenerate;

  newell(pos, poly, nrm);
  *axisOut = dominantAxis(nrm);

  double ext3 = 0.0;
  for (int i : poly) {
    const Vec3 d = pos[i] - pos[poly[0]];
    ext3 = std::max({ext3, std::fabs(static_cast<double>(d.x)),
                     std::fabs(static_cast<double>(d.y)),
                     std::fabs(static_cast<double>(d.z))});
  }
  if (std::fabs(nrm[*axisOut]) <= 1e-12 * ext3 * ext3) {
    // 法線がゼロ = 面積0。一直線なら描くものがないだけ、
    // そうでなければ蝶ネクタイ形のように面積が打ち消し合った自己交差
    return allCollinear(pos, poly) ? FaceResult::Degenerate
                                   : FaceResult::SelfIntersecting;
  }

  if (poly.size() == 3) {
    out->push_back({poly[0], poly[1], poly[2]});
    return FaceResult::Ok;
  }

  std::vector<P2> pt(poly.size());
  double extent = 0.0;
  for (size_t i = 0; i < poly.size(); ++i) {
    const Vec3& v = pos[poly[i]];
    if (*axisOut == 0) {
      pt[i] = {v.y, v.z};
    } else if (*axisOut == 1) {
      pt[i] = {v.z, v.x};
    } else {
      pt[i] = {v.x, v.y};
    }
    extent = std::max(
        {extent, std::fabs(pt[i].x - pt[0].x), std::fabs(pt[i].y - pt[0].y)});
  }
  const double eps = 1e-12 * extent * extent;  // 外積(長さ^2)の許容誤差
  const double tol = 1e-9 * extent;            // 座標(長さ)の許容誤差

  if (!isSimple(pt, eps, tol)) return FaceResult::SelfIntersecting;

  const double sign = nrm[*axisOut] > 0.0 ? 1.0 : -1.0;  // 反時計回りなら +1
  if (!earClip(pt, poly, sign, eps, out)) return FaceResult::NoEar;
  return FaceResult::Ok;
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

      std::vector<Tri> tris;
      int axis = 2;
      switch (triangulate(positions, face, &tris, &axis)) {
        case FaceResult::Ok:
          break;
        case FaceResult::Degenerate:
          continue;  // 面積0の多角形。描くものがないので出力しない
        case FaceResult::SelfIntersecting:
          fail(path, lineNo, "self-intersecting face");
          break;
        case FaceResult::NoEar:
          fail(path, lineNo, "cannot triangulate face (invalid polygon)");
          break;
      }
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
