#version 330

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;

struct Camera {
    vec3 position;
    float fov;
    float aspectRatio;
    vec3 front;
    vec3 up;
};

uniform Camera uCamera;

out vec3 rayDir;
out vec3 rayOrigin;
out vec2 texCoord;
// for more pixel ray samples, the max direction floating range
flat out vec3 vHorizontalRange;
flat out vec3 vVerticalRange;
uniform vec2 uFramebufferSize;
vec2 calcViewportSize() {
    // distance == 1.0
    float h = tan(uCamera.fov / 2.0);
    float height = 2.0 * h;
    float width = uCamera.aspectRatio * height;
    return vec2(width, height);
}

void main() {
    vec2 filmSize = calcViewportSize();
    vec3 w = normalize(-uCamera.front);
    vec3 u = normalize(cross(uCamera.up, w));
    vec3 v = cross(w, u);

    vHorizontalRange = u * 0.5 * filmSize.x / uFramebufferSize.x;
    vVerticalRange = v * 0.5 * filmSize.y / uFramebufferSize.y;
    vec3 lowerLeft =
        uCamera.position - 0.5 * u * filmSize.x - 0.5 * v * filmSize.y - w;
    rayOrigin = uCamera.position;
    rayDir = lowerLeft + u * aTexCoord.x * filmSize.x +
             v * aTexCoord.y * filmSize.y - rayOrigin;
    gl_Position = vec4(aPosition.x, aPosition.y, 0.0, 1.0);
    texCoord = aTexCoord;
}