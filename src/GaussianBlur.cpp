#include "GaussianBlur.hpp"

#include <glm/vec2.hpp>

#include "asset.hpp"
using namespace glm;

GaussianBlur::GaussianBlur(GLsizei width, GLsizei height)
    : m_shader{{{SHADER_DIR "/finalShader.vert", GL_VERTEX_SHADER},
                {SHADER_DIR "/gaussianBlurShader.frag", GL_FRAGMENT_SHADER}}},
      m_width{width},
      m_height{height} {}

void GaussianBlur::init(GLenum internalformat, GLenum format, GLenum type) {
    m_texs[0].init();
    m_texs[0].setup(m_width, m_height, internalformat, format, type, 0);
    m_texs[0].setSizeFilter(GL_LINEAR, GL_LINEAR);
    m_texs[0].setWrapFilter(GL_CLAMP_TO_EDGE);

    m_texs[1].init();
    m_texs[1].setup(m_width, m_height, internalformat, format, type, 0);
    m_texs[1].setSizeFilter(GL_LINEAR, GL_LINEAR);
    m_texs[1].setWrapFilter(GL_CLAMP_TO_EDGE);

    m_fbs[0].init();
    m_fbs[0].attachTexture(m_texs[0], GL_COLOR_ATTACHMENT0, 0);

    m_fbs[1].init();
    m_fbs[1].attachTexture(m_texs[1], GL_COLOR_ATTACHMENT0, 0);
}

void GaussianBlur::blur(Texture& tex, int blurKerSize) {
    m_fbs[0].bind();
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT);
    m_shader.use();

    m_shader.setTexture("uSrcTex", 0, tex);
    m_shader.setUniform("uDirection", vec2(1.0, 0.0));
    m_shader.setUniform("uBlurKerSize", blurKerSize);
    m_quad.draw();
    m_fbs[0].unbind();

    m_fbs[1].bind();
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT);

    m_shader.setTexture("uSrcTex", 0, m_texs[0]);
    m_shader.setUniform("uDirection", vec2(0.0, 1.0));
    m_shader.setUniform("uBlurKerSize", blurKerSize);
    m_quad.draw();

    glActiveTexture(GL_TEXTURE0);
    tex.bind();
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, m_width, m_height);
    tex.unbind();
    m_fbs[1].unbind();
}