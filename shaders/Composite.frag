#version 450

layout(set = 0, binding = 0) uniform sampler2D albedoSampler;
layout(set = 0, binding = 1) uniform sampler2D normalSampler;
layout(set = 0, binding = 2) uniform sampler2D depthSampler;
layout(set = 0, binding = 3) uniform sampler2D shadowSampler;

layout(location = 0) out vec4 outColor;


layout(push_constant) uniform PushConstants
{
    vec3 sunDirection;
    vec3 sunColor;
    vec3 ambientColor;
} push;


void main()
{
    vec2 uv =
        gl_FragCoord.xy /
        vec2(textureSize(albedoSampler,0));


    vec3 albedo =
        texture(albedoSampler,uv).rgb;


    vec4 normalSample =
        texture(normalSampler,uv);


    // empty GBuffer pixel
    if(normalSample.a == 0.0)
    {
        discard;
    }


    vec3 normal =
        normalize(
            normalSample.rgb * 2.0 - 1.0
        );


    float shadow =
        clamp(
            texture(shadowSampler,uv).r,
            0.0,
            1.0
        );


    float NdotL =
        max(
            dot(
                normal,
                normalize(push.sunDirection)
            ),
            0.0
        );


    vec3 direct =
        albedo *
        push.sunColor *
        NdotL *
        shadow;


    vec3 ambient =
        albedo *
        push.ambientColor;


    outColor =
        vec4(
            direct + ambient,
            1.0
        );
}