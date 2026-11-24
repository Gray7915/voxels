#version 450

layout(location = 0) in vec3 position;
layout(location = 3) in vec2 uv;

layout(push_constant) uniform Push {
    mat4 mvp;
} push;

layout(location = 0) out vec2 fragUV;

void main() {
    fragUV = uv;
    gl_Position = push.mvp * vec4(position, 1.0);
}