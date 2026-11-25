// Copyright [2026] yyamasak
// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <format>
#include <iostream>
#include <stdexcept>
#include <string>

#include "custom/Camera.hpp"
#include "custom/Matrix.hpp"
#include "custom/Mesh.hpp"
#include "custom/MotionMode.hpp"
#include "custom/Shader.hpp"
#include "custom/Texture.hpp"
#include "custom/Transform.hpp"
#include "custom/Vec.hpp"
#include "custom/WaveFrontObjectLoader.hpp"

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void keyCallback(GLFWwindow* window, int key, int scancode, int action,
                 int mods);
void processInput(GLFWwindow* window, float deltaTime);
void processCameraInput(GLFWwindow* window, float deltaTime);
void processObjectInput(GLFWwindow* window, float deltaTime);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void initialize() {
#ifdef __APPLE__
  // Cocoa経由だとcwdが変更され相対パスが崩れるのでヒント追加
  glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif
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
  Transform* transform;
  bool isCameraLocked = false;
  bool firstMouse = true;
  bool textureMode = false;
  float aspect = static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT);
  MotionMode::MotionModeValue motionMode = MotionMode::Camera;
};

int main(int ac, char** av) {
  GLFWwindow* window = nullptr;

  try {
    // 入力が不正ならウィンドウを開く前に失敗させる(エラーは下の catch で表示)
    std::string objPath = "resources/42.obj";
    if (ac > 1) objPath = av[1];
    ObjData obj = loadObj(objPath);

    initialize();
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "scop", NULL, NULL);
    if (window == NULL) {
      throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      throw std::runtime_error("Failed to initialize GLAD");
    }
    glEnable(GL_DEPTH_TEST);

    {
      Camera camera(ft_math::Vec3(5.0f, 0.0f, 0.0f),
                    ft_math::Vec3(0.0f, 1.0f, 0.0f), 180.0f, 0.0f);
      // モデルは原点中心に正規化済みなので、初期位置も原点(画面中央)
      Transform transform = Transform(ft_math::Vec3(0.0f), 0.0f, 2.5f);
      AppContext ctx{&camera, &transform};
      glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
      glfwSetWindowUserPointer(window, &ctx);
      glfwSetMouseButtonCallback(window, mouseButtonCallback);
      glfwSetCursorPosCallback(window, mouseCallback);
      glfwSetScrollCallback(window, scrollCallback);
      glfwSetKeyCallback(window, keyCallback);
      Shader shader("resources/glsl/sample.vs", "resources/glsl/sample.fs");
      shader.use();
      Mesh mesh;
      mesh.upload(obj);

      Texture tex("resources/awesomeface.bmp");
      float blendFactor = 0.0f;
      ft_math::Mat4 model = transform.getModelMatrix();
      ft_math::Mat4 view = camera.getViewMatrix();
      ft_math::Mat4 projection = ft_math::perspective(
          ft_math::radians(camera.zoom),
          ctx.aspect, 0.1f, 100.0f);

      float preTime = glfwGetTime();
      while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - preTime;
        processInput(window, deltaTime);
        preTime = currentTime;

        float target = ctx.textureMode ? 1.0f : 0.0f;
        blendFactor += (target - blendFactor) * deltaTime * 2.0f;
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (transform.autoRotate) transform.rotation += deltaTime * 1.0f;
        model = transform.getModelMatrix();
        view = camera.getViewMatrix();
        projection = ft_math::perspective(
            ft_math::radians(camera.zoom),
            ctx.aspect, 0.1f, 100.0f);

        tex.bind(0);

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
  } catch (const std::exception& e) {
    std::cerr << "[ERROR] " << e.what() << std::endl;
    if (window != nullptr) {
      glfwDestroyWindow(window);
    }
    glfwTerminate();
    return 1;
  } catch (...) {
    std::cerr << "[ERROR] unknown exception" << std::endl;
    if (window != nullptr) {
      glfwDestroyWindow(window);
    }
    glfwTerminate();
    return 1;
  }
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
  // 投影行列の縦横比もウィンドウに合わせる(固定だとリサイズで歪む)。
  // 最小化時は高さ0になるので0除算を避ける。
  AppContext* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
  if (ctx != nullptr && width > 0 && height > 0)
    ctx->aspect = static_cast<float>(width) / static_cast<float>(height);
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
  static double lastX = SCR_WIDTH / 2.0;
  static double lastY = SCR_HEIGHT / 2.0;

  AppContext* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
  if (ctx->isCameraLocked) {
    ctx->firstMouse = true;
    return;
  }
  if (ctx->firstMouse) {
    lastX = xpos;
    lastY = ypos;
    ctx->firstMouse = false;
  }

  double xoffset = xpos - lastX;
  double yoffset = lastY - ypos;  // ← Y軸は反転（上がプラス）
  lastX = xpos;
  lastY = ypos;

  ctx->camera->processMouseMovement(xoffset, yoffset);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
  (void)mods;
  AppContext* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    ctx->isCameraLocked = !ctx->isCameraLocked;
    // ロックされた瞬間、あるいは解除された瞬間に
    // 次の移動でワープしないよう firstMouse を true にしておく
    ctx->firstMouse = true;
  }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
  (void)xoffset;
  AppContext* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
  ctx->camera->processMouseScroll(yoffset);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action,
                 int mods) {
  (void)scancode;
  (void)mods;

  AppContext* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));

  if (key == GLFW_KEY_TAB && action == GLFW_RELEASE) {
    if (ctx->motionMode == MotionMode::Camera) {
      ctx->motionMode = MotionMode::Object;
    } else if (ctx->motionMode == MotionMode::Object) {
      ctx->motionMode = MotionMode::Camera;
    }
    const std::string title =
        "scop - Mode: " + std::string(MotionMode::toName(ctx->motionMode));
    glfwSetWindowTitle(window, title.c_str());
  }

  if (key == GLFW_KEY_T && action == GLFW_PRESS)
    ctx->textureMode = !(ctx->textureMode);
  // R: 自動回転の停止/再開(ユーザーが自動回転を制御できる)
  if (key == GLFW_KEY_R && action == GLFW_PRESS)
    ctx->transform->autoRotate = !(ctx->transform->autoRotate);
}

void processInput(GLFWwindow* window, float deltaTime) {
  AppContext* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  if (ctx->motionMode == MotionMode::Camera)
    processCameraInput(window, deltaTime);
  else if (ctx->motionMode == MotionMode::Object)
    processObjectInput(window, deltaTime);
}

void processCameraInput(GLFWwindow* window, float deltaTime) {
  AppContext* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    ctx->camera->processKeyboard(Movement::FORWARD, deltaTime);
  else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    ctx->camera->processKeyboard(Movement::BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    ctx->camera->processKeyboard(Movement::LEFT, deltaTime);
  else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    ctx->camera->processKeyboard(Movement::RIGHT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    ctx->camera->processKeyboard(Movement::DOWN, deltaTime);
  else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    ctx->camera->processKeyboard(Movement::UP, deltaTime);
}

void processObjectInput(GLFWwindow* window, float deltaTime) {
  AppContext* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    ctx->transform->move(ctx->camera->front, deltaTime);
  else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    ctx->transform->move(-ctx->camera->front, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    ctx->transform->move(-ctx->camera->right, deltaTime);
  else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    ctx->transform->move(ctx->camera->right, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    ctx->transform->move(-ctx->camera->up, deltaTime);
  else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    ctx->transform->move(ctx->camera->up, deltaTime);

  // 3軸まわりの回転: ↑↓=X軸, ←→=Y軸, Q/E=Z軸
  const float step = 1.5f * deltaTime;
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    ctx->transform->rotationX += step;
  else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    ctx->transform->rotationX -= step;
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    ctx->transform->rotation += step;
  else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    ctx->transform->rotation -= step;
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    ctx->transform->rotationZ += step;
  else if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    ctx->transform->rotationZ -= step;
}
