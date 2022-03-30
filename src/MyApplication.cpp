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
      m_sun_position(0, 10.0, 0),
      m_cam(vec3(-0.65, 0.4, 0), glm::vec3(0.0f, 1.0f, 0.0f), -11.5, -9.2) {
    m_scene.scale(vec3(0.01));

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
    ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)", m_cam.position.x,
                m_cam.position.y, m_cam.position.z);
    ImGui::Text("Camera Pitch: %.1f, Yaw: %.1f", m_cam.pitch, m_cam.yaw);
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
        m_sun_moved = true;
    }
    if (glfwGetKey(getWindow(), GLFW_KEY_DOWN) == GLFW_PRESS) {
        m_sun_position.x += 10.0 * getFrameDeltaTime();
        m_sun_moved = true;
    }
    if (glfwGetKey(getWindow(), GLFW_KEY_LEFT) == GLFW_PRESS) {
        m_sun_position.z += 10.0 * getFrameDeltaTime();
        m_sun_moved = true;
    }
    if (glfwGetKey(getWindow(), GLFW_KEY_RIGHT) == GLFW_PRESS) {
        m_sun_position.z -= 10.0 * getFrameDeltaTime();
        m_sun_moved = true;
    }
}
void MyApplication::loop() {
    if (glfwWindowShouldClose(getWindow()) ||
        glfwGetKey(getWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
        exit();
    bool light_rerendered_flag = false;
    // input interact
    cameraMove();
    sunMove();

    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(RGB_DIV_255(135, 206, 235), 1.0);
    checkError();

    glBindVertexArray(0);
    gui();

    if (light_rerendered_flag && m_sun_moved) m_sun_moved = false;
}
