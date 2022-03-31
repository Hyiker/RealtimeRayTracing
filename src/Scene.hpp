#ifndef SCENE_H
#define SCENE_H
#include <glad/glad.h>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Cube.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

// mesh level aabb bound box
struct BoundBox {
    glm::vec3 min;
    glm::vec3 max;
};
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    BoundBox aabb;
    bool operator==(const Vertex& v) const;
};
namespace std {
template <>
struct hash<Vertex> {
    size_t operator()(Vertex const& v) const;
};
}  // namespace std

struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float shininess;
    float ior;
    int illum;

    std::shared_ptr<Texture> ambientTex;
    std::shared_ptr<Texture> diffuseTex;
    std::shared_ptr<Texture> displacementTex;
    std::shared_ptr<Texture> normalTex;
    std::shared_ptr<Texture> specularTex;
    std::shared_ptr<Texture> alphaTex;
};
struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    Material material;
    BoundBox aabb;
    Cube cube;

    GLuint vao, vbo, ebo;
    void prepare();
    size_t countVertex() const;
    void draw(ShaderProgram& sp) const;
    void drawCubes(ShaderProgram& shader) const;
};
class Scene {
    std::vector<Mesh> m_meshes;
    glm::mat4 m_modelmat{1.0};
    glm::mat4 m_normalmat{1.0};
    bool m_modelmat_modified_flag{false};

   public:
    void scale(glm::vec3 ratio);
    void translate(glm::vec3 pos);
    glm::mat4 getModelMatrix() const { return m_modelmat; };
    glm::mat4 getNormalMatrix();
    size_t countMesh() const;
    size_t countVertex() const;
    void draw(ShaderProgram& sp) const;
    void drawCubes(ShaderProgram& shader) const;
    Scene(const std::string& path);
};

glm::mat4 getLightSpaceTransform(glm::vec3 lightPosition);

#endif /* SCENE_H */
