// Copyright [2026] yyamasak
#include <array>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "custom/WaveFrontObjectLoader.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

static const std::array<std::array<float, 3>, 9> kPalette = {{
    {1.0f, 0.3f, 0.3f},
    {0.3f, 1.0f, 0.3f},
    {0.3f, 0.3f, 1.0f},
    {1.0f, 1.0f, 0.3f},
    {1.0f, 0.3f, 1.0f},
    {0.3f, 1.0f, 1.0f},
    {1.0f, 0.6f, 0.2f},
    {0.6f, 0.2f, 1.0f},
    {0.2f, 0.8f, 0.6f},
}};

ObjData loadObj(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open())
    throw std::runtime_error("Cannot open OBJ file: " + path);

  std::vector<glm::vec3> positions;
  ObjData data;
  int faceIndex = 0;

  std::string line;
  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::string token;
    ss >> token;

    if (token == "v") {
      float x, y, z;
      ss >> x >> y >> z;
      glm::vec3 v(x, y, z);
      positions.push_back(v);
    } else if (token == "f") {
      std::vector<int> indices;
      std::string idx;
      while (ss >> idx) {
        int i = std::stoi(idx.substr(0, idx.find('/')));  // 1/1の形式に対応する
        indices.push_back(i > 0 ? i - 1
                                : (static_cast<int>(positions.size()) + i));
      }

      const auto& color = kPalette[faceIndex % kPalette.size()];

      // Fan triangulation
      for (size_t i = 1; i + 1 < indices.size(); ++i) {
        for (int vi : {indices[0], indices[i], indices[i + 1]}) {
          data.vertices.push_back(positions[vi].x);
          data.vertices.push_back(positions[vi].y);
          data.vertices.push_back(positions[vi].z);
          data.vertices.push_back(color[0]);
          data.vertices.push_back(color[1]);
          data.vertices.push_back(color[2]);
        }
      }
      ++faceIndex;
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

  return data;
}
