/**
 * MyApplication.cpp
 * Contributors:
 *      * Arthur Sonzogni (author)
 * Licence:
 *      * MIT
 */
#include "MyApplication.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/matrix.hpp>
#include <iostream>
#include <vector>

#include "Framebuffer.hpp"
#include "Helper.h"
#include "asset.hpp"
#include "glError.hpp"

using namespace std;
using namespace glm;

static void mouseCallback(GLFWwindow *window, double xposIn, double yposIn) {
    static bool firstMouse = true;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        firstMouse = true;
        return;
    }
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    MyApplication *myapp =
        static_cast<MyApplication *>(glfwGetWindowUserPointer(window));
    static float lastX = myapp->getWindowWidth() / 2.0;
    static float lastY = myapp->getWindowHeight() / 2.0;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;
    myapp->getCamera().processMouseMovement(xoffset, yoffset);
}

MyApplication::MyApplication(const string &path, int width, int height)
    : Application(width, height),
      m_scene(path),
      m_bp_shader{{SHADER_DIR "/blinnPhongShader.vert", GL_VERTEX_SHADER},
                  {SHADER_DIR "/blinnPhongShader.frag", GL_FRAGMENT_SHADER}},
      m_sun_position(-0.234011, 5.319334, -3.042968),
      m_cam(vec3(0, 0, 4.5), glm::vec3(0.0f, 1.0f, 0.0f), -90.f, 0.f) {
    glfwSetWindowUserPointer(getWindow(), this);
    glfwSetCursorPosCallback(getWindow(), mouseCallback);
}
void MyApplication::gui() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(
        ImVec2(float(getWindowWidth()) / 3.0, getWindowHeight()));
    ImGui::Begin("Debug Panel");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    // information
    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *version = glGetString(GL_VERSION);
    ImGui::Text("Renderer: %s", renderer);
    ImGui::Text("OpenGL Version: %s", version);

    // object information
    ImGui::Separator();
    ImGui::Text("Meshes: %lu, Vertices: %lu", m_scene.countMesh(),
                m_scene.countVertex());
    auto cam_pos = m_cam.getPosition();
    ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)", cam_pos.x, cam_pos.y,
                cam_pos.z);
    ImGui::Text("Sun Position: (%.1f, %.1f, %.1f)", m_sun_position.x,
                m_sun_position.y, m_sun_position.z);

    ImGui::End();
}
void MyApplication::cameraMove() {
    if (glfwGetKey(getWindow(), GLFW_KEY_W) == GLFW_PRESS)
        m_cam.processKeyboard(CameraMovement::FORWARD, getFrameDeltaTime());
    if (glfwGetKey(getWindow(), GLFW_KEY_S) == GLFW_PRESS)
        m_cam.processKeyboard(CameraMovement::BACKWARD, getFrameDeltaTime());
    if (glfwGetKey(getWindow(), GLFW_KEY_A) == GLFW_PRESS)
        m_cam.processKeyboard(CameraMovement::LEFT, getFrameDeltaTime());
    if (glfwGetKey(getWindow(), GLFW_KEY_D) == GLFW_PRESS)
        m_cam.processKeyboard(CameraMovement::RIGHT, getFrameDeltaTime());
}
void MyApplication::sunMove() {
    if (glfwGetKey(getWindow(), GLFW_KEY_UP) == GLFW_PRESS) {
        m_sun_position.x -= 10.0 * getFrameDeltaTime();
    }
    if (glfwGetKey(getWindow(), GLFW_KEY_DOWN) == GLFW_PRESS) {
        m_sun_position.x += 10.0 * getFrameDeltaTime();
    }
    if (glfwGetKey(getWindow(), GLFW_KEY_LEFT) == GLFW_PRESS) {
        m_sun_position.z += 10.0 * getFrameDeltaTime();
    }
    if (glfwGetKey(getWindow(), GLFW_KEY_RIGHT) == GLFW_PRESS) {
        m_sun_position.z -= 10.0 * getFrameDeltaTime();
    }
}
void MyApplication::loop() {
    if (glfwWindowShouldClose(getWindow()) ||
        glfwGetKey(getWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
        exit();
    // input
    cameraMove();
    sunMove();

    // rendering
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(RGB_DIV_255(135, 206, 235), 1.0);
    checkError();

    glEnable(GL_DEPTH_TEST);
    m_bp_shader.use();
    m_bp_shader.setUniform("uProjection", m_cam.getProjectionMatrix());
    m_bp_shader.setUniform("uView", m_cam.getViewMatrix());
    m_bp_shader.setUniform("uModel", m_scene.getModelMatrix());
    m_bp_shader.setUniform("uNormalTransform", m_scene.getNormalMatrix());
    m_bp_shader.setUniform("uCamPosition", m_cam.getPosition());
    m_bp_shader.setUniform("uSunPosition", m_sun_position);

    m_scene.draw(m_bp_shader);
    checkError();

    glBindVertexArray(0);
    gui();
}
