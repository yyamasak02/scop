// Copyright [2026] yyamasak
// clang-format off
#include <glad/glad.h>
// clang-format on
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "custom/ImageLoader.hpp"
#include "custom/Logger.hpp"

namespace ft_image {

static uint32_t readU32(const unsigned char* p) {
  return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
         (static_cast<uint32_t>(p[2]) << 16) |
         (static_cast<uint32_t>(p[3]) << 24);
}

static int32_t readI32(const unsigned char* p) {
  return static_cast<int32_t>(readU32(p));
}

static uint16_t readU16(const unsigned char* p) {
  return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}

ImageData loadImage(const std::string& image_path) {
  std::ifstream file(image_path, std::ios::binary);
  if (!file.is_open()) {
    Logger::error("texture file not found: " + image_path);
    return ImageData{};
  }

  unsigned char fileHeader[14];
  file.read(reinterpret_cast<char*>(fileHeader), 14);
  if (!file || fileHeader[0] != 'B' || fileHeader[1] != 'M') {
    Logger::error("texture is not a BMP: " + image_path);
    return ImageData{};
  }
  uint32_t dataOffset = readU32(fileHeader + 10);

  unsigned char dibSizeBytes[4];
  file.read(reinterpret_cast<char*>(dibSizeBytes), 4);
  uint32_t dibSize = readU32(dibSizeBytes);
  if (dibSize < 12) {
    Logger::error("texture has invalid DIB size: " + image_path);
    return ImageData{};
  }

  std::vector<unsigned char> dib(dibSize - 4);
  file.read(reinterpret_cast<char*>(dib.data()), dibSize - 4);
  if (!file) {
    Logger::error("failed to read texture DIB header: " + image_path);
    return ImageData{};
  }

  int32_t w = readI32(dib.data());
  int32_t h = readI32(dib.data() + 4);
  uint16_t bpp = readU16(dib.data() + 10);

  if (w <= 0 || h == 0) {
    Logger::error("texture has invalid dimensions: " + image_path);
    return ImageData{};
  }
  if (bpp != 24 && bpp != 32) {
    Logger::error("texture has unsupported bpp (" + std::to_string(bpp) +
                  "): " + image_path);
    return ImageData{};
  }

  ImageData imgData;
  imgData.width = w;
  imgData.bpp = bpp;
  bool topDown = (h < 0);
  imgData.height = topDown ? -h : h;
  int channels = bpp / 8;

  int rowSize = ((imgData.width * channels) + 3) & ~3;
  int rawSize = rowSize * imgData.height;

  file.seekg(dataOffset, std::ios::beg);
  std::vector<unsigned char> raw(rawSize);
  file.read(reinterpret_cast<char*>(raw.data()), rawSize);
  if (!file) {
    Logger::error("failed to read texture pixels: " + image_path);
    return ImageData{};
  }

  std::vector<unsigned char> pixels(static_cast<size_t>(imgData.width) *
                                    imgData.height * channels);
  for (int y = 0; y < imgData.height; ++y) {
    int srcRow = topDown ? (imgData.height - 1 - y) : y;
    std::memcpy(&pixels[static_cast<size_t>(y) * imgData.width * channels],
                &raw[static_cast<size_t>(srcRow) * rowSize],
                static_cast<size_t>(imgData.width) * channels);
  }

  imgData.format = (channels == 4) ? GL_BGRA : GL_BGR;
  imgData.internalFormat = (channels == 4) ? GL_RGBA : GL_RGB;
  imgData.data = std::move(pixels);
  return imgData;
}

}  // namespace ft_image
