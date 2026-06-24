#include "ObjLoader.h"

#include <array>
#include <fstream>
#include <sstream>
#include <stdexcept>

struct Vec3 {
  float x, y, z;
};

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

  std::vector<Vec3> positions;
  ObjData data;
  int faceIndex = 0;

  std::string line;
  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::string token;
    ss >> token;

    if (token == "v") {
      Vec3 v;
      ss >> v.x >> v.y >> v.z;
      positions.push_back(v);
    } else if (token == "f") {
      std::vector<int> indices;
      std::string idx;
      while (ss >> idx) {
        int i = std::stoi(idx.substr(0, idx.find('/')));
        indices.push_back(i > 0 ? i - 1 : (int)positions.size() + i);
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

  data.vertexCount = (int)data.vertices.size() / 6;

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
