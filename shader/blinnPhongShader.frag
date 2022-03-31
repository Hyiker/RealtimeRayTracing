#version 330

in vec3 position;
in vec3 normal;
in vec2 texCoord;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float shininess;
    float ior;
    int illum;
};

uniform Material material;
uniform vec3 uSunPosition;
uniform vec3 uCamPosition;

// output
out vec4 color;

void main(void) {
    vec3 ambient = material.ambient * 0.1;

    vec3 lightDir = normalize(uSunPosition - position);
    vec3 diffuse = max(dot(normal, lightDir), 0.0) * material.diffuse * 0.3;

    vec3 viewDir = normalize(uCamPosition - position);
    vec3 H = normalize(viewDir + lightDir);
    vec3 specular = material.specular *
                    pow(max(dot(H, normal), 0.0), material.shininess) * 1;

    color = vec4(ambient + diffuse + specular, 1.0);
    /*color = vec3(1,0,0);*/
}
