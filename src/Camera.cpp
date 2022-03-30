#include <Camera.hpp>
#include <iostream>
using namespace std;
void Camera::updateCameraVectors() {
    glm::vec3 fwd;
    fwd.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    fwd.y = sin(glm::radians(pitch));
    fwd.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(fwd);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(m_fov, m_aspect, m_znear, m_zfar);
}

glm::vec3 Camera::getPosition() const { return position; }

void Camera::processKeyboard(CameraMovement direction, float deltaTime) {
    float velocity = speed * deltaTime;
    if (direction == CameraMovement::FORWARD) position += front * velocity;
    if (direction == CameraMovement::BACKWARD) position -= front * velocity;
    if (direction == CameraMovement::LEFT) position -= right * velocity;
    if (direction == CameraMovement::RIGHT) position += right * velocity;
}

void Camera::processMouseMovement(float xoffset, float yoffset,
                                  GLboolean constrainpitch) {
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    yaw = yaw > 360.0 ? yaw - 360.0 : yaw;
    yaw = yaw < -360.0 ? yaw + 360.0 : yaw;
    pitch += yoffset;

    if (constrainpitch) {
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
    }

    updateCameraVectors();
}