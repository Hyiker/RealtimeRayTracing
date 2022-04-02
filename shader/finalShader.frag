#version 330
in vec2 texCoord;
out vec4 color;
uniform sampler2D uRenderTexture;
void main() {
    vec3 col = texture(uRenderTexture, texCoord).rgb;

    color = vec4(col, 1.0);
}