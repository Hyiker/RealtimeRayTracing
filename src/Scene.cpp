#include "Scene.hpp"

#include <glad/glad.h>

#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#include <glm/gtx/hash.hpp>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <limits>
#include <unordered_map>

#include "glError.hpp"
#include "tiny_obj_loader.h"

using namespace std;
using namespace glm;
using namespace tinyobj;
size_t hash<Vertex>::operator()(Vertex const& v) const {
    return ((hash<glm::vec3>()(v.position) ^
             (hash<glm::vec3>()(v.normal) << 1)) >>
            1) ^
           (hash<glm::vec2>()(v.texCoord) << 1);
}
namespace fs = std::filesystem;
void Mesh::prepare() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (GLvoid*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (GLvoid*)(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (GLvoid*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

size_t Mesh::countVertex() const { return vertices.size(); }

void Mesh::draw(ShaderProgram& sp) const {
    glBindVertexArray(vao);

    sp.setUniform("material.ambient", material.ambient);
    sp.setUniform("material.diffuse", material.diffuse);
    sp.setUniform("material.specular", material.specular);
    sp.setUniform("material.shininess", material.shininess);
    sp.setUniform("material.ior", material.ior);
    sp.setUniform("material.illum", material.illum);

    if (material.ambientTex)
        sp.setTexture("material.ambientTex", 0, material.ambientTex->getId());
    if (material.diffuseTex)
        sp.setTexture("material.diffuseTex", 1, material.diffuseTex->getId());
    if (material.specularTex)
        sp.setTexture("material.specularTex", 2, material.specularTex->getId());
    if (material.displacementTex)
        sp.setTexture("material.displacementTex", 3,
                      material.displacementTex->getId());

    if (material.normalTex)
        sp.setTexture("material.normalTex", 4, material.normalTex->getId());

    glDrawElements(GL_TRIANGLES, static_cast<GLuint>(indices.size()),
                   GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

void Mesh::drawCubes(ShaderProgram& shader) const {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    cube.draw(shader, aabb.min, aabb.max);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Scene::scale(glm::vec3 ratio) {
    m_modelmat_modified_flag = true;
    m_modelmat = glm::scale(m_modelmat, ratio);
}

void Scene::translate(glm::vec3 pos) {
    m_modelmat_modified_flag = true;
    m_modelmat = glm::translate(m_modelmat, pos);
}

glm::mat4 Scene::getNormalMatrix() {
    if (m_modelmat_modified_flag) {
        m_normalmat = glm::transpose(glm::inverse(m_modelmat));
        m_modelmat_modified_flag = true;
    }
    return m_normalmat;
}

size_t Scene::countMesh() const { return m_meshes.size(); }

size_t Scene::countVertex() const {
    size_t cnt = 0;
    for (const auto& mesh : m_meshes) {
        cnt += mesh.countVertex();
    }
    return cnt;
}

void Scene::draw(ShaderProgram& sp) const {
    for (const auto& mesh : m_meshes) {
        mesh.draw(sp);
    }
}

void Scene::drawCubes(ShaderProgram& shader) const {
    for (auto& mesh : m_meshes) {
        mesh.drawCubes(shader);
    }
}

static shared_ptr<Texture> loadTexture(
    unordered_map<string, shared_ptr<Texture>>& uniqueTexture, string parentDir,
    string texName) {
    if (texName.length() == 0) return nullptr;
    if (uniqueTexture.count(texName)) return uniqueTexture[texName];
    shared_ptr<Texture> tex = make_shared<Texture>();

    int ncomp;
    int width, height;
    string path = (parentDir + '/' + texName);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &ncomp, 0);
    if (!data) {
        throw runtime_error("Failed to read texture " + path);
    }
    GLenum format;
    switch (ncomp) {
        case 1:
            format = GL_RED;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
        default:
            throw runtime_error("Unsupported tex format " + to_string(ncomp));
            break;
    }
    tex->init();
    tex->setup(data, width, height, GL_SRGB, format, GL_UNSIGNED_BYTE, 0);
    tex->setSizeFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    tex->setWrapFilter(GL_REPEAT);
    tex->generateMipmap();

    stbi_image_free(data);
    uniqueTexture[texName] = tex;
    return tex;
}

static vector<Mesh> readObject(const std::string& parentPath,
                               const std::string& modelPath) {
    ObjReader reader;
    ObjReaderConfig config;

    if (!reader.ParseFromFile(modelPath, config)) {
        if (!reader.Error().empty()) {
            throw runtime_error("TinyObjReader: " + reader.Error());
        }
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();
    vector<Mesh> meshes;

    unordered_map<string, std::shared_ptr<Texture>> uniqueTexture;
    unordered_map<Vertex, unsigned int> uniqueVertices;
    for (auto& shape : shapes) {
        Mesh mesh;
        Material material;
        vec3 bb_min{numeric_limits<float>::infinity()},
            bb_max{-numeric_limits<float>::infinity()};
        // one material for one mesh
        if (shape.mesh.material_ids.size()) {
            auto& mat = materials[shape.mesh.material_ids[0]];
            for (int i = 0; i < 3; i++) {
                material.ambient[i] = mat.ambient[i];
                material.diffuse[i] = mat.diffuse[i];
                material.specular[i] = mat.specular[i];
            }
            material.shininess = mat.shininess;
            material.ior = mat.ior;
            material.illum = mat.illum;

            material.ambientTex =
                loadTexture(uniqueTexture, parentPath, mat.ambient_texname);
            material.diffuseTex =
                loadTexture(uniqueTexture, parentPath, mat.diffuse_texname);
            material.specularTex =
                loadTexture(uniqueTexture, parentPath, mat.specular_texname);
            material.displacementTex = loadTexture(uniqueTexture, parentPath,
                                                   mat.displacement_texname);
            material.normalTex =
                loadTexture(uniqueTexture, parentPath, mat.normal_texname);
        }
        for (int i = 0; i < shape.mesh.indices.size(); i++) {
            Vertex vertex;
            auto& idx = shape.mesh.indices[i];
            // access to vertex
            vertex.position.x =
                attrib.vertices[3 * size_t(idx.vertex_index) + 0];
            vertex.position.y =
                attrib.vertices[3 * size_t(idx.vertex_index) + 1];
            vertex.position.z =
                attrib.vertices[3 * size_t(idx.vertex_index) + 2];
            bb_max = glm::max(vertex.position, bb_max);
            bb_min = glm::min(vertex.position, bb_min);

            // Check if `normal_index` is zero or positive. negative = no
            // normal data
            if (idx.normal_index >= 0) {
                vertex.normal.x =
                    attrib.normals[3 * size_t(idx.normal_index) + 0];
                vertex.normal.y =
                    attrib.normals[3 * size_t(idx.normal_index) + 1];
                vertex.normal.z =
                    attrib.normals[3 * size_t(idx.normal_index) + 2];
            }

            // Check if `texcoord_index` is zero or positive. negative = no
            // texcoord data
            if (idx.texcoord_index >= 0) {
                vertex.texCoord.x =
                    attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                vertex.texCoord.y =
                    attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
            }
            if (!uniqueVertices.count(vertex)) {
                uniqueVertices[vertex] = mesh.vertices.size();
                mesh.vertices.push_back(move(vertex));
            }
            mesh.indices.push_back(uniqueVertices[vertex]);
        }
        for (auto& vertex : mesh.vertices) {
            vertex.aabb.min = bb_min;
            vertex.aabb.max = bb_max;
        }
        mesh.aabb.min = bb_min;
        mesh.aabb.max = bb_max;
        mesh.material = move(material);
        meshes.push_back(mesh);
    }
    return meshes;
}
Scene::Scene(const std::string& path) {
    fs::path abspath = fs::absolute(path);
    cout << "loading model from " << abspath << '\n';
    stbi_set_flip_vertically_on_load(true);
    m_meshes = readObject(abspath.parent_path(), abspath);

    // prepare mesh vao ebo
    for (auto& mesh : m_meshes) {
        mesh.prepare();
    }
}

glm::mat4 getLightSpaceTransform(glm::vec3 lightPosition) {
    vec3 target(0, 0, 0);
    vec3 dir = normalize(target - lightPosition);
    vec3 up{0.0f, 1.0f, 0.0f};
    if (dir.x == 0.0f && dir.z == 0.0f) up = glm::vec3(1.0f, 0.0f, 0.0f);
    float bound = 1.0;
    return ortho(-bound, bound, -bound, bound, -1.5f, 1.5f) *
           lookAt(normalize(lightPosition), target, up);
}

bool Vertex::operator==(const Vertex& v) const {
    return position == v.position && normal == v.normal &&
           texCoord == v.texCoord;
}
