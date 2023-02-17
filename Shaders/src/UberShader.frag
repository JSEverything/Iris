#version 450 core
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : require
#include "common.glsl"

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (set = 0, binding = 0) uniform CameraData1 {
    CameraData camera;
};

layout (set = 0, binding = 1) uniform LightData1 {
    LightData lightsMeta;
};

layout (std140, set = 0, binding = 2) readonly buffer LightStorage1 {
    Light[] lights;
};

layout (set = 1, binding = 0) uniform sampler2D textures[];

layout (location = 0) out vec4 outColor;
layout (location = 1) out uint outID;

layout (push_constant) uniform PushConstants1 {
    PushConstants pc;
};

vec3 Phong(vec3 color) {
    vec3 cameraPos = camera.position.xyz;
    vec3 normal = normalize(inNormal);
    float specularIntensity = 0.5f;

    vec3 total = { 0.f, 0.f, 0.f };

    for (uint i = 0; i < lightsMeta.count; i++) {
        vec3 lightColor = lights[i].color.xyz;
        if (lights[i].flags.x == 0) // point
        {
            vec3 lightDirection = normalize(lights[i].position.xyz - inPosition);
            float diffuse = max(dot(inNormal, lightDirection), 0.f);

            vec3 viewDirection = normalize(cameraPos - inPosition);
            vec3 reflectionDirection = reflect(-lightDirection, viewDirection);
            float specularAmount = pow(max(dot(viewDirection, reflectionDirection), 0), 8);
            float specular = specularIntensity * specularAmount;

            total += color * lightColor * (diffuse + specular);
        }
        else if (lights[i].flags.x == 1) // directional
        {
            vec3 lightDirection = normalize(- lights[i].position.xyz);
            float diffuse = max(dot(inNormal, lightDirection), 0.f);

            vec3 viewDirection = normalize(cameraPos - inPosition);
            vec3 reflectionDirection = reflect(-lightDirection, viewDirection);
            float specularAmount = pow(max(dot(viewDirection, reflectionDirection), 0), 8);
            float specular = specularIntensity * specularAmount;

            total += color * lightColor * (diffuse + specular);
        }
        else if (lights[i].flags.x == 1) // spot
        {
            vec3 lightDirection = normalize(lights[i].rotation.xyz);
            float diffuse = max(dot(inNormal, lightDirection), 0.f);

            vec3 viewDirection = normalize(cameraPos - inPosition);
            vec3 reflectionDirection = reflect(-lightDirection, viewDirection);
            float specularAmount = pow(max(dot(viewDirection, reflectionDirection), 0), 8);
            float specular = specularIntensity * specularAmount;

            total += color * lightColor * (diffuse + specular);
        }
    }

    return total + color * 0.1f;
}

void main()
{
    outID = pc.objectID + 1;

    vec4 color;

    ivec2 size = textureSize(textures[pc.textureID], 0);
    if (size.x + size.y <= 2) {
        color = vec4(0.3f, 0.3f, 0.3f, 1.f); // empty texture
    } else {
        color = vec4(texture(textures[pc.textureID], inUV).rgb, 1.f);
    }

    outColor = vec4(Phong(color.rgb), 1.f);
}