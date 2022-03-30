/**
 * Application.hpp
 * Contributors:
 *      * Arthur Sonzogni (author)
 * Licence:
 *      * MIT
 */

#include "Application.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <stdexcept>

#include "glError.hpp"

using namespace std;

Application* currentApplication = NULL;

Application& Application::getInstance() {
    if (currentApplication)
        return *currentApplication;
    else
        throw std::runtime_error("There is no current Application");
}

Application::Application(int width, int height)
    : state(stateReady), width(width), height(height), title("Application") {
    currentApplication = this;

    cout << "[Info] GLFW initialisation" << endl;

    // initialize the GLFW library
    if (!glfwInit()) {
        throw std::runtime_error("Couldn't init GLFW");
    }

    // setting the opengl version
    int major = 3;
    int minor = 3;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    // create the window
    window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Couldn't create a window");
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Fail to initialize GLAD");
    }

    // ImGui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    glCheckError(__FILE__, __LINE__);

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    (void)io;
    // opengl configuration
    glEnable(GL_DEPTH_TEST);  // enable depth-testing
    glDepthFunc(
        GL_LESS);  // depth-testing interprets a smaller value as "closer"

    // vsync
    // glfwSwapInterval(false);
}

GLFWwindow* Application::getWindow() const { return window; }

void Application::exit() { state = stateExit; }

float Application::getFrameDeltaTime() const { return deltaTime; }

float Application::getTime() const { return time; }

void Application::run() {
    state = stateRun;

    // Make the window's context current
    glfwMakeContextCurrent(window);

    time = glfwGetTime();

    while (state == stateRun) {
        // compute new time and delta time
        float t = glfwGetTime();
        deltaTime = t - time;
        time = t;
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // detech window related changes
        detectWindowDimensionChange();

        // execute the frame code
        loop();

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap Front and Back buffers (double buffering)
        glfwSwapBuffers(window);

        // Pool and process events
        glfwPollEvents();
    }
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void Application::detectWindowDimensionChange() {
    int w, h;
    glfwGetWindowSize(getWindow(), &w, &h);
    dimensionChanged = (w != width || h != height);
    if (dimensionChanged) {
        width = w;
        height = h;
        glViewport(0, 0, width, height);
    }
}

void Application::loop() { cout << "[INFO] : loop" << endl; }

#ifdef __APPLE__
int Application::getFramebufferWidth() {
    int width, _;
    glfwGetFramebufferSize(getWindow(), &width, &_);
    return width;
}

int Application::getFramebufferHeight() {
    int _, height;
    glfwGetFramebufferSize(getWindow(), &_, &height);
    return height;
}
int Application::getWindowWidth() { return width; }

int Application::getWindowHeight() { return height; }
#else
int Application::getWidth() { return width; }

int Application::getHeight() { return height; }
#endif

float Application::getWindowRatio() { return float(width) / float(height); }

bool Application::windowDimensionChanged() { return dimensionChanged; }
