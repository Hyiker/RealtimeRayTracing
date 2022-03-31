#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Camera {
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_world_up;
    float m_yaw;
    float m_pitch;
    float m_speed{2.5f};
    float m_sensitivity{0.2f};
    float m_znear{0.01f};
    float m_zfar{50.f};
    float m_fov{float(M_PI) / 3.0f};
    float m_aspect;

   public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f,
           float pitch = 0.0f, float aspect = 4.f / 3.f)
        : m_position(position),
          m_front(glm::vec3(0.0f, 0.0f, -1.0f)),
          m_world_up{up},
          m_yaw(yaw),
          m_pitch(pitch),
          m_aspect{aspect} {
        updateCameraVectors();
    }

    glm::mat4 getViewMatrix() const;

    glm::mat4 getProjectionMatrix() const;

    glm::vec3 getPosition() const { return m_position; }

    void processKeyboard(CameraMovement direction, float deltaTime);

    void processMouseMovement(float xoffset, float yoffset,
                              GLboolean constrainpitch = true);

   private:
    void updateCameraVectors();
};

#endif /* CAMERA_H */
