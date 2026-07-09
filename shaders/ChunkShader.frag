#version 450

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUv;
layout(location = 3) in vec3 fragColor;
layout(location = 4) in float fragAo;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D atlasSampler;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionView;
    vec4 ambientLightColor;   // w = intensity
    vec3 lightPosition;
    vec4 lightColor;          // w = intensity
    vec4 cameraPosition;      // xyz used
} ubo;

const float PI = 3.14159265359;

// --- Hardcoded "dirt" material for now ---
// Swap this block out once you add a per-vertex materialID attribute.
const float ROUGHNESS = 0.85;
const float METALNESS = 0.0;
// ------------------------------------------

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return a2 / max(denom, 0.0000001);
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float geometrySmith(float NdotV, float NdotL, float roughness) {
    float ggxV = geometrySchlickGGX(NdotV, roughness);
    float ggxL = geometrySchlickGGX(NdotL, roughness);
    return ggxV * ggxL;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec3 albedo = texture(atlasSampler, fragUv).rgb * fragColor;

    vec3 N = normalize(fragNormal);
    vec3 V = normalize(ubo.cameraPosition.xyz - fragWorldPos);

    vec3 toLight = ubo.lightPosition - fragWorldPos;
    float lightDist = length(toLight);
    vec3 L = toLight / lightDist;
    vec3 H = normalize(V + L);

    float NdotV = max(dot(N, V), 0.0001);
    float NdotL = max(dot(N, L), 0.0);

    vec3 F0 = mix(vec3(0.04), albedo, METALNESS);

    float NDF = distributionGGX(N, H, ROUGHNESS);
    float G = geometrySmith(NdotV, NdotL, ROUGHNESS);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - METALNESS);

    vec3 numerator = NDF * G * F;
    float denom = 4.0 * NdotV * NdotL + 0.0001;
    vec3 specular = numerator / denom;

    // Attenuation intentionally omitted: lightPosition{0,70,0} reads as a "sun"
    // stand-in, and inverse-square falloff would make distant terrain go dark.
    // Add `1.0 / (lightDist * lightDist)` back in if this is meant to behave
    // like a true local point light instead.
    vec3 lightRadiance = ubo.lightColor.rgb * ubo.lightColor.w;
    vec3 direct = (kD * albedo / PI + specular) * lightRadiance * NdotL;

    // Cheap ambient stand-in for IBL: sky/ground blend by normal, occluded by baked AO.
    vec3 skyColor = vec3(0.4, 0.55, 0.75);
    vec3 groundColor = vec3(0.2, 0.18, 0.15);
    vec3 hemisphereAmbient = mix(groundColor, skyColor, N.y * 0.5 + 0.5);
    vec3 ambient = hemisphereAmbient * ubo.ambientLightColor.rgb * ubo.ambientLightColor.w
                   * albedo * fragAo;

    // Light touch of AO on direct light too (contact shadowing), not full multiply.
    float directAo = mix(1.0, fragAo, 0.3);

    vec3 color = direct * directAo + ambient;

    // Reinhard tonemap + gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, 1.0);
}
