#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 4) in float ao;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out float fragAO;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    vec4 crackRegion;
} push;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionViewMatrix;
    vec4 ambientLightColor;
    vec3 lightPosition;
    vec4 lightColor;
} ubo;

void main() {
    vec4 positionWorld =
        push.modelMatrix * vec4(position, 1.0);

    gl_Position =
        ubo.projectionViewMatrix * positionWorld;

    fragColor = color;
    fragUV = uv;
    fragAO = ao;
}