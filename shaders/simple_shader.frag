#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragWorldPos;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragUV;
layout(location = 4) flat in ivec3 blockPos;

layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projectionViewMatrix;
  vec4 ambientLightColor; // w is intensity
  vec3 lightPosition;
  vec4 lightColor;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
    ivec4 lookingAt;
} push;

void main()
{
    vec3 directionToLight = ubo.lightPosition - fragWorldPos;
    float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared

    vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
    vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    vec3 diffuseLight = lightColor * max(dot(normalize(fragNormal), normalize(directionToLight)), 0);

    vec4 texColor = texture(texSampler, fragUV);
    vec3 highlightColor = vec3(0.9, 0.8, 0.8);
    
    outColor = texColor * vec4(fragColor, 1.0);
}
