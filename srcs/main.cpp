// Copyright [2026] yyamasak
// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "custom/Camera.hpp"
#include "custom/Mesh.hpp"
#include "custom/Shader.hpp"
#include "custom/WaveFrontObjectLoader.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, Camera& camera, float deltaTime);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void initialize() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

struct AppContext {
  Camera* camera;
};

int main() {
  initialize();
  GLFWwindow* window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "scop", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    glfwTerminate();
    return -1;
  }
  glEnable(GL_DEPTH_TEST);

  {
    Camera camera(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
                  180.0f, 0.0f);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    AppContext ctx{&camera};
    glfwSetWindowUserPointer(window, &ctx);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    ObjData obj = loadObj("resources/42.obj");
    Shader shader("resources/glsl/sample.vs", "resources/glsl/sample.fs");
    shader.setFloat("yMin", obj.yMin);
    shader.setFloat("yMax", obj.yMax);
    shader.setFloat("zMin", obj.zMin);
    shader.setFloat("zMax", obj.zMax);
    Mesh mesh;
    mesh.upload(obj);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int texWidth, texHeight, nChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* imgData = stbi_load("resources/awesomeface.png", &texWidth,
                                       &texHeight, &nChannels, 0);
    if (imgData) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, imgData);
      glGenerateMipmap(GL_TEXTURE_2D);
    } else {
      std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(imgData);

    float blendFactor = 0.0f;
    bool textureMode = false;
    bool tKeyWasPressed = false;

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(
        glm::radians(camera.zoom),
        static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f,
        100.0f);

    float preTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
      float currentTime = glfwGetTime();
      float deltaTime = currentTime - preTime;
      processInput(window, camera, deltaTime);
      preTime = currentTime;

      bool tKeyNow = glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS;
      if (tKeyNow && !tKeyWasPressed) textureMode = !textureMode;
      tKeyWasPressed = tKeyNow;

      float target = textureMode ? 1.0f : 0.0f;
      blendFactor += (target - blendFactor) * deltaTime * 2.0f;
      glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      float angle = static_cast<float>(glfwGetTime());
      model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
      view = camera.GetViewMatrix();
      projection = glm::perspective(
          glm::radians(camera.zoom),
          static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f,
          100.0f);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);

      shader.use();
      shader.setMat4("model", model);
      shader.setMat4("view", view);
      shader.setMat4("projection", projection);
      shader.setFloat("blendFactor", blendFactor);
      shader.setInt("tex", 0);

      mesh.draw();

      glfwSwapBuffers(window);
      glfwPollEvents();
    }
  }
  glfwTerminate();
  return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
  static bool firstMouse = true;
  static double lastX = SCR_WIDTH / 2.0;
  static double lastY = SCR_HEIGHT / 2.0;

  AppContext* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  double xoffset = xpos - lastX;
  double yoffset = lastY - ypos;  // ← Y軸は反転（上がプラス）
  lastX = xpos;
  lastY = ypos;

  ctx->camera->ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
  AppContext* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
  ctx->camera->ProcessMouseScroll(yoffset);
}

void processInput(GLFWwindow* window, Camera& camera, float deltaTime) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.ProcessKeyboard(FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.ProcessKeyboard(LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.ProcessKeyboard(RIGHT, deltaTime);
}
