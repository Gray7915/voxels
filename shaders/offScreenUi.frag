// offScreenUi.frag
#version 450

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionView;
    vec4 ambientLightColor;
    vec3 lightPosition;
    vec4 lightColor;
    vec4 cameraPosition;
    vec4 fogColor;
    float fogStart;
    float fogEnd;
} ubo;

layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(1, 1, 0.0, 1.0); 
}