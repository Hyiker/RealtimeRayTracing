#ifndef QUAD_H
#define QUAD_H
#include <glad/glad.h>
class Quad {
   private:
    GLuint vao, vbo;

   public:
    Quad();
    void draw() const;
    ~Quad();
};

#endif /* QUAD_H */
