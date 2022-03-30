#ifndef TEXTURE_H
#define TEXTURE_H
#include <glad/glad.h>

#include <string>
class Texture {
    GLuint m_id;
    GLsizei width, height;

   public:
    void init() { glGenTextures(1, &m_id); }
    void bind() const { glBindTexture(GL_TEXTURE_2D, m_id); }
    void unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }
    GLuint getId() const { return m_id; };
    GLenum getType() const { return GL_TEXTURE_2D; }
    int getMipmapLevels() const {
        unsigned int lvl = 0;
        int width = this->width, height = this->height;
        while ((width | height) >> 1) {
            width >>= 1;
            height >>= 1;
            lvl++;
        }
        return lvl;
    }
    void generateMipmap() {
        bind();
        glGenerateMipmap(GL_TEXTURE_2D);
        unbind();
    }
    void setSizeFilter(GLenum min_filter, GLenum mag_filter) {
        bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
        unbind();
    }
    void setWrapFilter(GLenum filter) {
        bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, filter);
        unbind();
    }
    void setClampToBorderFilter(GLfloat* borderColor) {
        bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        unbind();
    }
    void setup(GLsizei width, GLsizei height, GLenum internalformat,
               GLenum format, GLenum type, GLsizei level) {
        setup(nullptr, width, height, internalformat, format, type, level);
    }
    void setup(unsigned char* data, GLsizei width, GLsizei height,
               GLenum internalformat, GLenum format, GLenum type, GLint level) {
        bind();
        this->width = width;
        this->height = height;
        glTexImage2D(GL_TEXTURE_2D, level, internalformat, width, height, 0,
                     format, type, data);

        unbind();
    }
};
#endif /* TEXTURE_H */
