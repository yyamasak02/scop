#include <cstdlib>
#include <iostream>

#include <GLFW/glfw3.h>

static void renderTriangle() {
  glBegin(GL_TRIANGLES);
  glColor3f(1.0f, 0.2f, 0.2f);
  glVertex2f(-0.6f, -0.5f);

  glColor3f(0.2f, 1.0f, 0.2f);
  glVertex2f(0.6f, -0.5f);

  glColor3f(0.2f, 0.4f, 1.0f);
  glVertex2f(0.0f, 0.6f);
  glEnd();
}

/**
 * 1. glfwInit
 * 2. windowHint
 * 3. window作成
 * 4. context設定
 * 5. swapInterval(property)
 * 6. eventLoop
 */

int main() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return EXIT_FAILURE;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

  GLFWwindow *window = glfwCreateWindow(800, 600, "scop", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  while (!glfwWindowShouldClose(window)) {
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glClearColor(0.08f, 0.10f, 0.14f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    renderTriangle();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return EXIT_SUCCESS;
}