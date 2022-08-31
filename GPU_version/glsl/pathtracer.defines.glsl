#version 330 core

// Uniforms
uniform vec3 u_Eye;
uniform vec3 u_Forward;
uniform vec3 u_Right;
uniform vec3 u_Up;

uniform int u_Iterations;
uniform vec2 u_ScreenDims;

uniform sampler2D u_AccumImg;
uniform sampler2D u_EnvironmentMap;
uniform sampler2D u_BlueNoiseTex;

// Varyings
in vec3 fs_Pos;
in vec2 fs_UV;
out vec4 out_Col;

// Numeric constants
#define PI               3.14159265358979323
#define TWO_PI           6.28318530717958648
#define FOUR_PI          12.5663706143591729
#define INV_PI           0.31830988618379067
#define INV_TWO_PI       0.15915494309
#define INV_FOUR_PI      0.07957747154594767
#define PI_OVER_TWO      1.57079632679489662
#define ONE_THIRD        0.33333333333333333
#define E                2.71828182845904524
#define INFINITY         1000000.0
#define OneMinusEpsilon  0.99999994

// Path tracer recursion limit
#define MAX_DEPTH 4

// Light source types
#define SPOT_LIGHT  -2
#define POINT_LIGHT -1
#define AREA_LIGHT   0

// Material types
#define DIFFUSE_REFL    1
#define SPEC_REFL       2
#define SPEC_TRANS      3
#define GLASS           4
#define MICROFACET_REFL 5

// Data structures
struct Ray {
    vec3 origin;
    vec3 direction;
};

struct Material {
    vec3  albedo;
    float roughness;
    int   type; // Refer to the #defines above
};

struct Intersection {
    float t;
    vec3  nor;
    vec3  Le; // Emitted light
    int   obj_ID;
    Material material;
};

struct Transform {
    mat4 T;
    mat4 invT;
    mat3 invTransT;
};

struct Sphere {
    vec3 pos;
    float radius;
    vec3 Le;
    int ID;
    Material material;
};

struct Rectangle {
    vec3 pos;
    vec3 nor;
    vec2 halfSideLengths; // Dist from center to horizontal/vertical edge
    vec3 Le;
    int ID;
    Material material;
};

struct Box {
    vec3 minCorner;
    vec3 maxCorner;
    Transform transform;
    vec3 Le;
    int ID;
    Material material;
};


// Functions
float AbsDot(vec3 a, vec3 b) {
    return abs(dot(a, b));
}

float CosTheta(vec3 w) { return w.z; }
float Cos2Theta(vec3 w) { return w.z * w.z; }
float AbsCosTheta(vec3 w) { return abs(w.z); }
float Sin2Theta(vec3 w) {
    return max(0.f, 1.f - Cos2Theta(w));
}
float SinTheta(vec3 w) { return sqrt(Sin2Theta(w)); }
float TanTheta(vec3 w) { return SinTheta(w) / CosTheta(w); }

float Tan2Theta(vec3 w) {
    return Sin2Theta(w) / Cos2Theta(w);
}

float CosPhi(vec3 w) {
    float sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 1 : clamp(w.x / sinTheta, -1.f, 1.f);
}
float SinPhi(vec3 w) {
    float sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 0 : clamp(w.y / sinTheta, -1.f, 1.f);
}
float Cos2Phi(vec3 w) { return CosPhi(w) * CosPhi(w); }
float Sin2Phi(vec3 w) { return SinPhi(w) * SinPhi(w); }

Ray SpawnRay(vec3 pos, vec3 wi) {
    return Ray(pos + wi * 0.0001, wi);
}

mat4 translate(vec3 t) {
    return mat4(1,0,0,0,
                0,1,0,0,
                0,0,1,0,
                t.x, t.y, t.z, 1);
}

float radians(float deg) {
    return deg * PI / 180.f;
}

mat4 rotateX(float rad) {
    return mat4(1,0,0,0,
                0,cos(rad),sin(rad),0,
                0,-sin(rad),cos(rad),0,
                0,0,0,1);
}

mat4 rotateY(float rad) {
    return mat4(cos(rad),0,-sin(rad),0,
                0,1,0,0,
                sin(rad),0,cos(rad),0,
                0,0,0,1);
}


mat4 rotateZ(float rad) {
    return mat4(cos(rad),sin(rad),0,0,
                -sin(rad),cos(rad),0,0,
                0,0,1,0,
                0,0,0,1);
}

mat4 scale(vec3 s) {
    return mat4(s.x,0,0,0,
                0,s.y,0,0,
                0,0,s.z,0,
                0,0,0,1);
}

Transform makeTransform(vec3 t, vec3 euler, vec3 s) {
    mat4 T = translate(t)
             * rotateX(radians(euler.x))
             * rotateY(radians(euler.y))
             * rotateZ(radians(euler.z))
             * scale(s);

    return Transform(T, inverse(T), inverse(transpose(mat3(T))));
}

bool Refract(vec3 wi, vec3 n, float eta, out vec3 wt) {
    // Compute cos theta using Snell's law
    float cosThetaI = dot(n, wi);
    float sin2ThetaI = max(float(0), float(1 - cosThetaI * cosThetaI));
    float sin2ThetaT = eta * eta * sin2ThetaI;

    // Handle total internal reflection for transmission
    if (sin2ThetaT >= 1) return false;
    float cosThetaT = sqrt(1 - sin2ThetaT);
    wt = eta * -wi + (eta * cosThetaI - cosThetaT) * n;
    return true;
}

vec3 Faceforward(vec3 n, vec3 v) {
    return (dot(n, v) < 0.f) ? -n : n;
}

bool SameHemisphere(vec3 w, vec3 wp) {
    return w.z * wp.z > 0;
}

void coordinateSystem(in vec3 v1, out vec3 v2, out vec3 v3) {
    if (abs(v1.x) > abs(v1.y))
            v2 = vec3(-v1.z, 0, v1.x) / sqrt(v1.x * v1.x + v1.z * v1.z);
        else
            v2 = vec3(0, v1.z, -v1.y) / sqrt(v1.y * v1.y + v1.z * v1.z);
        v3 = cross(v1, v2);
}

// Compute a 3x3 rotation matrix that orients a vector
// defined in local tangent space with the world-space
// coordinate system.
mat3 LocalToWorld(vec3 nor) {
    vec3 tan, bit;
    coordinateSystem(nor, tan, bit);
    return mat3(tan, bit, nor);
}

const vec2 normalize_uv = vec2(0.1591, 0.3183);
vec2 sampleSphericalMap(vec3 v) {
    // U is in the range [-PI, PI], V is [-PI/2, PI/2]
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    // Convert UV to [-0.5, 0.5] in U&V
    uv *= normalize_uv;
    // Convert UV to [0, 1]
    uv += 0.5;
    return uv;
}

