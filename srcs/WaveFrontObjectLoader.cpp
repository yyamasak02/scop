// Copyright [2026] yyamasak
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "custom/Vec.hpp"
#include "custom/WaveFrontObjectLoader.hpp"

ObjData loadObj(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open())
    throw std::runtime_error("Cannot open OBJ file: " + path);

  std::vector<ft_math::Vec3> positions;
  ObjData data;

  std::string line;
  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::string token;
    ss >> token;

    if (token == "v") {
      float x, y, z;
      ss >> x >> y >> z;
      ft_math::Vec3 v(x, y, z);
      positions.push_back(v);
    } else if (token == "f") {
      std::vector<int> indices;
      std::string idx;
      while (ss >> idx) {
        int i = std::stoi(idx.substr(0, idx.find('/')));  // 1/1の形式に対応する
        indices.push_back(i > 0 ? i - 1
                                : (static_cast<int>(positions.size()) + i));
      }

      // Fan triangulation
      for (size_t i = 1; i + 1 < indices.size(); ++i) {
        for (int vi : {indices[0], indices[i], indices[i + 1]}) {
          data.vertices.push_back(positions[vi].x);
          data.vertices.push_back(positions[vi].y);
          data.vertices.push_back(positions[vi].z);
          data.vertices.push_back(0.0f);
          data.vertices.push_back(0.0f);
          data.vertices.push_back(0.0f);
        }
      }
    }
  }

  data.vertexCount = (static_cast<int>(data.vertices.size())) / 6;

  // Center the mesh at its centroid
  float cx = 0, cy = 0, cz = 0;
  for (int i = 0; i < data.vertexCount; ++i) {
    cx += data.vertices[i * 6];
    cy += data.vertices[i * 6 + 1];
    cz += data.vertices[i * 6 + 2];
  }
  cx /= data.vertexCount;
  cy /= data.vertexCount;
  cz /= data.vertexCount;

  for (int i = 0; i < data.vertexCount; ++i) {
    data.vertices[i * 6] -= cx;
    data.vertices[i * 6 + 1] -= cy;
    data.vertices[i * 6 + 2] -= cz;
  }

  if (data.vertices.size() >= 2) {
    data.yMin = data.yMax = data.vertices[1];
    data.zMin = data.zMax = data.vertices[2];
  }

  for (int i = 0; i < data.vertexCount; ++i) {
    float y = data.vertices[i * 6 + 1];
    float z = data.vertices[i * 6 + 2];
    data.yMin = std::min(data.yMin, y);
    data.yMax = std::max(data.yMax, y);
    data.zMin = std::min(data.zMin, z);
    data.zMax = std::max(data.zMax, z);
  }
  for (int i = 0; i < data.vertexCount; ++i) {
    float y = data.vertices[i * 6 + 1];
    float grey = 0.1f + ((y - data.yMin) / (data.yMax - data.yMin)) * 0.4f;
    data.vertices[i * 6 + 3] = grey;
    data.vertices[i * 6 + 4] = grey;
    data.vertices[i * 6 + 5] = grey;
  }
  return data;
}
