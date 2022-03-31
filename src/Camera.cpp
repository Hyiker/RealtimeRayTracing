#include <Camera.hpp>
#include <iostream>
using namespace std;
void Camera::updateCameraVectors() {
    glm::vec3 fwd;
    fwd.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    fwd.y = sin(glm::radians(m_pitch));
    fwd.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(fwd);
    m_right = glm::normalize(glm::cross(m_front, m_world_up));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(m_fov, m_aspect, m_znear, m_zfar);
}

void Camera::processKeyboard(CameraMovement direction, float deltaTime) {
    float velocity = m_speed * deltaTime;
    if (direction == CameraMovement::FORWARD) m_position += m_front * velocity;
    if (direction == CameraMovement::BACKWARD) m_position -= m_front * velocity;
    if (direction == CameraMovement::LEFT) m_position -= m_right * velocity;
    if (direction == CameraMovement::RIGHT) m_position += m_right * velocity;
}

void Camera::processMouseMovement(float xoffset, float yoffset,
                                  GLboolean constrainpitch) {
    xoffset *= m_sensitivity;
    yoffset *= m_sensitivity;

    m_yaw += xoffset;
    m_yaw = m_yaw > 360.0 ? m_yaw - 360.0 : m_yaw;
    m_yaw = m_yaw < -360.0 ? m_yaw + 360.0 : m_yaw;
    m_pitch += yoffset;

    if (constrainpitch) {
        if (m_pitch > 89.0f) m_pitch = 89.0f;
        if (m_pitch < -89.0f) m_pitch = -89.0f;
    }

    updateCameraVectors();
}