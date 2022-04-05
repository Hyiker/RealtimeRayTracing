#version 330

layout(location = 0) out vec4 oColor0;
layout(location = 1) out vec4 oColor1;

#define RGB_DIV255(r, g, b) (r / 255.0), (g / 255.0), (b / 255.0)
#define SKY_BLUE 11, 119, 199
#define BLACK 0, 0, 0
in vec3 rayDir;
in vec3 rayOrigin;
in vec2 texCoord;
vec3 origin;
vec3 dir;

uniform int uLightBounceCount;
uniform int uLightSamples;

#define DIFFUSE 1
#define METALIC 2
#define GLASS 3

struct Material {
    bool isLight;
    vec3 color;
    int type;
};

struct Intersection {
    Material material;
    vec3 position;
    vec3 normal;
};

struct Sphere {
    vec3 o;
    float r;
    Material material;
};

struct AreaLight {
    float h;
    float r;
    Material material;
};
#define EPS 1e-3
#define PI 3.14159265359
float hasIntersectWithAreaLight(in vec3 origin, in vec3 dir, in AreaLight al,
                                inout Intersection inter) {
    float t = (al.h - origin.y) / dir.y;
    if (t < 0.0) {
        return -1.0;
    }
    vec3 coord = origin + t * dir;
    if (length(coord.xz) > al.r) {
        return -1.0;
    }
    inter.position = coord;
    inter.normal = vec3(0, dir.y > 0.0 ? -1.0 : 1.0, 0);
    inter.material = al.material;
    return t;
}

float hasIntersectWithSphere(in vec3 origin, in vec3 dir, in Sphere sphere,
                             inout Intersection inter) {
    vec3 oc = origin - sphere.o;
    float a = dot(dir, dir);  // 2
    float b = 2.0 * dot(dir, oc);
    float c = dot(oc, oc) - sphere.r * sphere.r;
    float discriminate = b * b - 4 * a * c;
    if (discriminate < 0) {
        return -1.0;
    } else {
        float t1 = (-b - sqrt(discriminate)) / (2.0 * a);
        float t2 = (-b + sqrt(discriminate)) / (2.0 * a);
        inter.material = sphere.material;
        // use t1 > EPS to prevent "startup" collision
        float t = t1 >= EPS ? t1 : t2;
        if (t < 0.0) return -1.0;
        inter.position = origin + dir * t;
        inter.normal = normalize(inter.position - sphere.o);
        return t;
    }
}

#define N_GEOMS 5
// microfacet material
Material mfWhite = Material(false, vec3(10.0), DIFFUSE);
Material mfGreen = Material(false, vec3(0.1, 0.6, 0.03), DIFFUSE);
// metalic white
Material metalicWhite = Material(false, vec3(1.0), METALIC);
// glass
Material glassWhite = Material(false, vec3(1.0), GLASS);
Material mfBlue = Material(false, vec3(RGB_DIV255(11, 119, 199)), DIFFUSE);
// light material
Material lWhite = Material(true, vec3(10.0), DIFFUSE);

// geometry primitives
Sphere sp1 = Sphere(vec3(0, 0, -1), 0.5, mfGreen);
Sphere sp3 = Sphere(vec3(-1.5, 0, -0.5), 0.5, metalicWhite);
Sphere sp4 = Sphere(vec3(-0.6, 0, 1.5), 0.5, glassWhite);
Sphere sp2 = Sphere(vec3(0.0, -1000.5, -1.0), 1000.0, mfBlue);
AreaLight light = AreaLight(2.0, 0.25, lWhite);
bool hasIntersect(in vec3 origin, in vec3 dir,
                  inout Intersection intersection) {
    Intersection inter[N_GEOMS];
    float t[N_GEOMS];
    t[0] = hasIntersectWithSphere(origin, dir, sp1, inter[0]);
    t[1] = hasIntersectWithSphere(origin, dir, sp2, inter[1]);
    t[2] = hasIntersectWithAreaLight(origin, dir, light, inter[2]);
    t[3] = hasIntersectWithSphere(origin, dir, sp3, inter[3]);
    t[4] = hasIntersectWithSphere(origin, dir, sp4, inter[4]);
    float t_min = -1.0;
    bool has_inter = false;
    for (int i = 0; i < N_GEOMS; i++) {
        if (t[i] >= 0.0 && (!has_inter || t[i] < t_min)) {
            t_min = t[i];
            intersection = inter[i];
            has_inter = true;
        }
    }
    return t_min > 0.0;
}

uniform vec3 uRand3;
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}
vec2 random2(vec2 p) {
    p = vec2(random(p), random(p * uRand3.xy * 100.0));

    return -1.0 + 2.0 * p;
}
vec3 random3(vec3 p) {
    p = vec3(dot(p, vec3(127.1, 311.7, 74.7)),
             dot(p, vec3(269.5, 183.3, 246.1)),
             dot(p, vec3(113.5, 271.9, 124.6)));

    return -1.0 + 2.0 * fract(sin(p) * 43758.5453123);
}

// modified from
// https://github.com/LWJGL/lwjgl3-demos/blob/main/res/org/lwjgl/demo/opengl/raytracing/randomCommon.glsl

vec3 sampleSphereUniform(vec3 rand) {
    float phi = (rand.x + 1.0) * PI;  // [-1..1) -> [0..2*PI)

    // inverse transform sampling
    float theta = acos(rand.y);
    float x = cos(phi) * sin(theta);
    float z = sin(phi) * sin(theta);
    float y = cos(theta);
    return vec3(x, y, z);
}
#define INV_2_PI 0.15915494
#define INV_PI 0.31830989
vec3 sampleHemisphereUniform(vec3 rand, vec3 n, out float pdf) {
    vec3 ph = sampleSphereUniform(rand);
    ph.z = abs(ph.z);
    pdf = INV_2_PI;

    vec3 tangent = normalize(rand);
    vec3 bitangent = normalize(cross(tangent, n));
    tangent = normalize(cross(bitangent, n));
    mat3 TBN = mat3(tangent, bitangent, n);
    vec3 dir = normalize(TBN * ph);

    return dir;
}

vec3 sampleHemisphereCosine(vec3 seed, vec3 n, out float pdf) {
    vec3 ss = vec3(0.0, 0.0, 1.0) + sampleSphereUniform(seed);
    float z = sqrt(1 - ss.x * ss.x - ss.y * ss.y);
    vec3 ph = vec3(ss.x, ss.y, z);

    vec3 tangent = normalize(seed);
    vec3 bitangent = normalize(cross(tangent, n));
    tangent = normalize(cross(bitangent, n));
    mat3 TBN = mat3(tangent, bitangent, n);
    vec3 dir = normalize(TBN * ph);

    pdf = z * INV_PI;
    return dir;
}

float fresnelApprox(float cosThetaI, float n1, float n2) {
    float r0 = (n1 - n2) / (n1 + n2);
    r0 *= r0;
    return r0 + (1.0 - r0) * pow(1.0 - cosThetaI, 5.0);
}

vec3 randSeed;
vec3 recursivePathTracing(vec3 origin, vec3 dir) {
    vec3 rayColor = vec3(0.0);
    vec3 rayBrightness = vec3(1.0);
    float p_RR = 0.9;

    for (int i = 0; i < uLightBounceCount; i++) {
        if (random(randSeed.xy) > p_RR) break;
        Intersection inter;
        if (!hasIntersect(origin, dir, inter)) {
            break;
        }
        Material material = inter.material;
        if (material.isLight) {
            rayColor = material.color * rayBrightness;
            break;
        } else {
            float cosine = 1.0;
            float pdf = 1.0;
            vec3 brdf = material.color;
            // add a slight offset from hit position to prevent artifacts
            // https://computergraphics.stackexchange.com/questions/7789/weird-artifacts-in-my-ray-tracer
            origin = inter.position + inter.normal * EPS;
            randSeed = random3(randSeed);
            if (material.type == DIFFUSE) {
                brdf *= INV_PI;
                dir = normalize(sampleHemisphereCosine(
                    randSeed, normalize(inter.normal), pdf));
                cosine = dot(dir, inter.normal);
            } else if (material.type == METALIC) {
                dir = reflect(dir, inter.normal);
                cosine = dot(dir, inter.normal);
            } else if (material.type == GLASS) {
                float si = dot(dir, inter.normal);
                float eta = 0.66;
                if (si < 0.0) {
                    float rd = random(randSeed.xy) + 1.0;
                    rd /= 2.0;
                    // incident
                    if (rd <= fresnelApprox(normalize(dot(-dir, inter.normal)),
                                            1.0, 1 / eta)) {
                        dir = reflect(dir, inter.normal);
                        cosine = dot(dir, inter.normal);
                    } else {
                        dir = refract(dir, inter.normal, eta);
                        origin = inter.position - inter.normal * EPS;
                        cosine = dot(dir, -inter.normal);
                    }
                } else {
                    // exitent
                    dir = refract(dir, -inter.normal, 1 / eta);
                    cosine = abs(dot(dir, inter.normal));
                }
            }
            cosine = max(cosine, 1e-5);
            pdf = max(pdf, 1e-5);
            rayBrightness *= brdf * cosine / p_RR / pdf;
        }
    }
    return rayColor;
}

flat in vec3 vHorizontalRange;
flat in vec3 vVerticalRange;
uniform int uPingpong;
uniform sampler2D uLastFrame;
void main() {
    origin = rayOrigin;
    vec3 res = vec3(0.0);
    vec2 uvRand = random2(texCoord + uRand3.xy);
    dir = normalize(rayDir);
    for (int i = 0; i < uLightSamples; i++) {
        randSeed = random3(vec3(texCoord + uRand3.xy, uRand3.z * dir.z + i));
        res += recursivePathTracing(origin, dir + uvRand.x * vHorizontalRange +
                                                uvRand.y * vVerticalRange);
        uvRand = random2(uvRand);
    }
    res += texture(uLastFrame, texCoord).rgb;
    if (uPingpong == 0) {
        oColor0 = vec4(res, 1.0);
    } else {
        oColor1 = vec4(res, 1.0);
    }
}