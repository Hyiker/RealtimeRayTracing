#ifndef CUBE_H
#define CUBE_H
#include <glad/glad.h>

#include <glm/vec3.hpp>

#include "Shader.hpp"
class Cube {
   private:
    GLuint vao, vbo;

   public:
    Cube();
    void draw(ShaderProgram& shader, glm::vec3 aabb_min,
              glm::vec3 aabb_max) const;
};

#endif /* CUBE_H */
