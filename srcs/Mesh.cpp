// Copyright [2026] yyamasak
// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <custom/Mesh.hpp>

// TODO(yyamasak) サイズが現状固定値なので、Nに対応
Mesh::Mesh() {
  glGenVertexArrays(1, &this->VAO);
  glGenBuffers(1, &this->VBO);
}

Mesh::~Mesh() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
}

void Mesh::upload(ObjData& objData) {
  glBindVertexArray(this->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
  glBufferData(GL_ARRAY_BUFFER, objData.vertices.size() * sizeof(float),
               objData.vertices.data(), GL_STATIC_DRAW);

  // position: location 0
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        reinterpret_cast<void*>(0));
  glEnableVertexAttribArray(0);
  // color: location 1
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        reinterpret_cast<void*>(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  this->vertexCount = objData.vertices.size() / 6;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void Mesh::draw() {
  glBindVertexArray(this->VAO);
  glDrawArrays(GL_TRIANGLES, 0, this->vertexCount);
}
