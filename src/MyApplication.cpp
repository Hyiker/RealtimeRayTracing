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
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/matrix.hpp>
#include <iostream>
#include <map>
#include <random>
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
    myapp->resetRayTracer();
}

MyApplication::MyApplication(int width, int height)
    : Application(width, height),
      m_rtrt_shader{{SHADER_DIR "/rtrtShader.vert", GL_VERTEX_SHADER},
                    {SHADER_DIR "/rtrtShader.frag", GL_FRAGMENT_SHADER}},
      m_final_shader{{SHADER_DIR "/finalShader.vert", GL_VERTEX_SHADER},
                     {SHADER_DIR "/finalShader.frag", GL_FRAGMENT_SHADER}},
      m_cam(vec3(0, 0, 4.5), glm::vec3(0.0f, 1.0f, 0.0f), -90.f, 0.f) {
    glfwSetWindowUserPointer(getWindow(), this);
    glfwSetCursorPosCallback(getWindow(), mouseCallback);

    for (int i = 0; i < 2; i++) {
        m_screen_texture[i].init();
        m_screen_texture[i].setup(getFramebufferWidth(), getFramebufferHeight(),
                                  GL_RGB16F, GL_RGBA, GL_FLOAT, 0);
        m_screen_texture[i].setSizeFilter(GL_LINEAR, GL_LINEAR);
    }
    m_fbo.init();
    m_fbo.attachTexture(m_screen_texture[0], GL_COLOR_ATTACHMENT0, 0);
    m_fbo.attachTexture(m_screen_texture[1], GL_COLOR_ATTACHMENT1, 0);
    m_fbo.bind();
    GLuint attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);
    checkError();
    m_fbo.unbind();
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

    // cam information
    ImGui::Separator();
    auto cam_pos = m_cam.getPosition();
    ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)", cam_pos.x, cam_pos.y,
                cam_pos.z);

    // raytracer settings
    ImGui::Separator();
    ImGui::TextUnformatted(
        "Press up/down arrow to adjust SPP\nLeft/right arrow to adjust Max "
        "Bounce");
    ImGui::SliderInt("Samples per Pixel(SPP) per Frame", &m_light_samples, 1,
                     4);
    ImGui::SliderInt("Light Bounce", &m_light_bounce, 0, 200);
    ImGui::Text("Current SPP: %lld", m_spp);

    ImGui::End();
}
static const map<int, CameraMovement> MOUSE_KEYMAP = {
    {GLFW_KEY_W, CameraMovement::FORWARD},
    {GLFW_KEY_S, CameraMovement::BACKWARD},
    {GLFW_KEY_A, CameraMovement::LEFT},
    {GLFW_KEY_D, CameraMovement::RIGHT},
};
void MyApplication::cameraMove() {
    for (auto const &[k, v] : MOUSE_KEYMAP) {
        if (glfwGetKey(getWindow(), k) == GLFW_PRESS) {
            m_cam.processKeyboard(v, getFrameDeltaTime());
            resetRayTracer();
        }
    }
}

void MyApplication::lightParamAjust() {
    if (glfwGetKey(getWindow(), GLFW_KEY_UP) == GLFW_PRESS)
        m_light_samples = std::min(m_light_samples + 1, 4);
    if (glfwGetKey(getWindow(), GLFW_KEY_DOWN) == GLFW_PRESS)
        m_light_samples = std::max(m_light_samples - 1, 1);

    if (glfwGetKey(getWindow(), GLFW_KEY_LEFT) == GLFW_PRESS)
        m_light_bounce = std::max(m_light_bounce - 1, 0);
    if (glfwGetKey(getWindow(), GLFW_KEY_RIGHT) == GLFW_PRESS)
        m_light_bounce = std::min(m_light_bounce + 1, 200);
}
static glm::vec3 randomVec3() {
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_real_distribution<> distr(0.0, 1.0);
    return vec3(distr(eng), distr(eng), distr(eng));
}
void MyApplication::loop() {
    if (glfwWindowShouldClose(getWindow()) ||
        glfwGetKey(getWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
        exit();
    // input
    cameraMove();
    lightParamAjust();

    // rendering
    checkError();
    static int pingpong = 0;

    m_fbo.bind();
    glDisable(GL_DEPTH_TEST);
    if (m_cam_move_flag) {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(GL_RGBA_BLACK);
        m_cam_move_flag = false;
    }

    m_rtrt_shader.use();
    m_rtrt_shader.setUniform("uCamera.position", m_cam.getPosition());
    m_rtrt_shader.setUniform("uCamera.fov", m_cam.getFov());
    m_rtrt_shader.setUniform("uCamera.aspectRatio", m_cam.getAspectRatio());
    m_rtrt_shader.setUniform("uCamera.front", m_cam.getFront());
    m_rtrt_shader.setUniform("uCamera.up", m_cam.getUp());
    m_rtrt_shader.setUniform("uLightBounceCount", m_light_bounce);
    m_rtrt_shader.setUniform("uLightSamples", m_light_samples);
    m_rtrt_shader.setUniform("uFramebufferSize", vec2(getFramebufferWidth(),
                                                      getFramebufferHeight()));
    m_rtrt_shader.setTexture("uLastFrame", 0, m_screen_texture[pingpong]);
    m_rtrt_shader.setUniform("uPingpong", pingpong ^ 1);

    m_rtrt_shader.setUniform("uRand3", randomVec3());
    m_spp += m_light_samples;
    m_quad.draw();

    m_fbo.unbind();
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(GL_RGBA_BLACK);
    m_final_shader.use();
    m_final_shader.setTexture("uRenderTexture", 0,
                              m_screen_texture[pingpong ^ 1]);
    m_final_shader.setUniform("uSPP", std::max(1, (int)m_spp));
    m_quad.draw();
    checkError();
    pingpong = pingpong ^ 1;
    glBindVertexArray(0);
    gui();
}
