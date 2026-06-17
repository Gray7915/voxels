#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 2) in vec2 fragUV;

layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main()
{
    vec4 texColor = texture(texSampler, fragUV);
    outColor = texColor * vec4(fragColor, 1.0);
}
