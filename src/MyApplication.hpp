/**
 * MyApplication.hpp skeleton
 * Contributors:
 *      * Arthur Sonzogni
 * Licence:
 *      * MIT
 */

#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include <Scene.hpp>
#include <string>
#include <vector>

#include "Application.hpp"
#include "Camera.hpp"
#include "Framebuffer.hpp"
#include "Quad.hpp"
#include "Shader.hpp"
class MyApplication : public Application {
    ShaderProgram m_rtrt_shader;
    ShaderProgram m_final_shader;

    Framebuffer m_fbo;
    Texture m_screen_texture[2];

    Quad m_quad;
    Camera m_cam;
    int m_light_bounce{90}, m_light_samples{1};
    long long m_spp{0};
    bool m_cam_move_flag{false};

   public:
    MyApplication(int width = 640, int height = 480);
    Camera &getCamera() { return m_cam; }
    void resetRayTracer() {
        m_spp = 0;
        m_cam_move_flag = true;
    }

   protected:
    void gui();
    virtual void loop();

   private:
    void cameraMove();
    void lightParamAjust();
};

#endif /* MYAPPLICATION_H */
