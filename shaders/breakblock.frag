#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in float fragAO;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D atlasSampler;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    vec4 crackRegion;
} push;

void main() {
    vec2 uv = push.crackRegion.xy + fragUV * push.crackRegion.zw;
    vec4 crack = texture(atlasSampler, uv);
    if (crack.a < 0.01) discard;
    outColor = vec4(crack.rgb, 1.0);
}