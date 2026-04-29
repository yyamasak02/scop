#include <cstdlib>
#include <iostream>

#include <GLFW/glfw3.h>

int main() {
  glfwInit();

  GLFWwindow *window = glfwCreateWindow(800, 600, "scop",nullptr, nullptr);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
}