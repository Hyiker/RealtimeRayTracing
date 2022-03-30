#include "Cube.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
using namespace std;
using namespace glm;

void Cube::draw(ShaderProgram& shader, glm::vec3 aabb_min,
                glm::vec3 aabb_max) const {
    glBindVertexArray(vao);
    glEnable(GL_DEPTH_TEST);
    shader.setUniform("uCubeModel", translate((aabb_min + aabb_max) * 0.5) *
                                        scale((aabb_max - aabb_min) * 0.5));
    for (int i = 0; i < 24; i += 4)
        glDrawArrays(GL_LINE_LOOP, i, 4);
    glBindVertexArray(0);
}

Cube::Cube() {
    constexpr GLfloat g_vertex_buffer_data[] = {
        1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,

        1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,
        -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,

        1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,
        -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f,

        1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,

        1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f,
        1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f,
    };
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data),
                 g_vertex_buffer_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindVertexArray(0);
}
