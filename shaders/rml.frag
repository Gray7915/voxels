#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(set = 0, binding = 0) uniform sampler2D fontTexture;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    vec2 translation;
    vec2 screenSize;
    int useTexture;
} push;

void main() {
    if (push.useTexture == 2) {
        // Colour texture (images, decorators)
        vec4 texColor = texture(fontTexture, fragTexCoord);
        outColor = fragColor * texColor;
    } else if (push.useTexture == 1) {
        // Font/glyph texture (alpha mask)
        vec4 texColor = texture(fontTexture, fragTexCoord);
        outColor = vec4(fragColor.rgb, fragColor.a * texColor.r);
    } else {
        // No texture
        outColor = fragColor;
    }
}