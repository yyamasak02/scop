// Copyright [2026] yyamasak
#pragma once

#include <glad/glad.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "custom/Matrix.hpp"

class Shader {
 public:
  unsigned int ID;
  // constructor generates the shader on the fly
  // ------------------------------------------------------------------------
  Shader(const char* vertexPath, const char* fragmentPath);
  // activate the shader
  // ------------------------------------------------------------------------
  void use() const;
  // utility uniform functions
  // ------------------------------------------------------------------------
  void setBool(const std::string& name, bool value) const;
  // ------------------------------------------------------------------------
  void setInt(const std::string& name, int value) const;
  // ------------------------------------------------------------------------
  void setFloat(const std::string& name, float value) const;
  // ------------------------------------------------------------------------
  void setVec2(const std::string& name, float x, float y) const;
  // ------------------------------------------------------------------------
  void setVec3(const std::string& name, float x, float y, float z) const;
  // ------------------------------------------------------------------------
  void setVec4(const std::string& name, float x, float y, float z,
               float w) const;
  // ------------------------------------------------------------------------
  void setMat4(const std::string& name, const ft_math::Mat4& mat) const;

 private:
  // utility function for checking shader compilation/linking errors.
  // ------------------------------------------------------------------------
  void checkCompileErrors(GLuint shader, std::string type);
};