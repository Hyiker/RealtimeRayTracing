#ifndef COLORHELPER_H
#define COLORHELPER_H

#define RGB_DIV_255(r, g, b) (r / 255.f), (g / 255.f), (b / 255.f)
#define RGBA_DIV_255(r, g, b, a) \
    (r / 255.f), (g / 255.f), (b / 255.f), (a / 255.f)
#define GL_RGB_BLACK 0, 0, 0
#define GL_RGBA_BLACK GL_RGB_BLACK, 0
#endif /* COLORHELPER_H */
