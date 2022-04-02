#version 330
in vec2 texCoord;
out vec4 color;
uniform sampler2D uRenderTexture;
uniform int uSPP;
void main() {
    vec3 col = texture(uRenderTexture, texCoord).rgb;

    col = pow(col / float(uSPP), vec3(1 / 2.2));

    color = vec4(col, 1.0);
}