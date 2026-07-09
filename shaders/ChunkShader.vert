#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 4) in float ao;

layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragUv;
layout(location = 3) out vec3 fragColor;
layout(location = 4) out float fragAo;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionView;
    vec4 ambientLightColor;   // w = intensity
    vec3 lightPosition;
    vec4 lightColor;          // w = intensity
    vec4 cameraPosition;      // xyz used
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main() {
    vec4 worldPos = push.modelMatrix * vec4(position, 1.0);
    gl_Position = ubo.projectionView * worldPos;

    fragWorldPos = worldPos.xyz;
    fragNormal = normalize(mat3(push.normalMatrix) * normal);
    fragUv = uv;
    fragColor = color;
    fragAo = ao;
}
