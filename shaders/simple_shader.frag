#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragWorldPos;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragUV;
layout(location = 4) in float fragAO;

layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

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

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main()
{
    // --- lighting (unchanged) ---
    vec3 directionToLight = ubo.lightPosition - fragWorldPos;
    float attenuation = 1.0 / dot(directionToLight, directionToLight);

    vec3 lightColor    = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
    vec3 ambientLight  = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    vec3 diffuseLight  = lightColor * max(dot(normalize(fragNormal), normalize(directionToLight)), 0);

    vec4 texColor = texture(texSampler, fragUV);
    texColor.rgb *= fragAO;

    if (texColor.a < 0.5)
        discard;

    vec3 litColor = texColor.rgb * fragColor * (ambientLight + diffuseLight);

    // --- fog ---
    float dist      = length(ubo.cameraPosition.xyz - fragWorldPos);
    float linear    = clamp((ubo.fogEnd - dist) / (ubo.fogEnd - ubo.fogStart), 0.0, 1.0);
    float fogFactor = linear * linear;
    vec3 finalColor = mix(ubo.fogColor.xyz, litColor, fogFactor);

    outColor = vec4(finalColor, texColor.a);
}