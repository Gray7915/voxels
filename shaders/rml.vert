#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(push_constant) uniform PushConstants {
    vec2 translation;
    vec2 screenSize;
    int useTexture;
} push;

void main() {
    vec2 pos = inPosition + push.translation;
    vec2 ndc;
    ndc.x = (pos.x / push.screenSize.x) * 2.0 - 1.0;
    ndc.y = (pos.y / push.screenSize.y) * 2.0 - 1.0;
    gl_Position = vec4(ndc, 0.0, 1.0);
    fragColor = inColor; // UNORM handles 0-255 to 0-1 conversion
    fragTexCoord = inTexCoord;
}