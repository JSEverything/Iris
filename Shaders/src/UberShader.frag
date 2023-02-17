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

float specularIntensity = 0.5f;
float ambientLight = 0.1f;

vec3 PointLight(vec3 albedo, Light light, vec3 cameraPos) {
    vec3 lightVec = light.position.xyz - inPosition;
    float dist = length(lightVec);
    float a = 1.f; // attenuation parameters
    float b = 2.f;
    float intensity = 1.f / (a * dist * dist + b * dist + 1.f);

    vec3 lightDirection = normalize(lightVec);
    float diffuse = max(dot(inNormal, lightDirection), 0.f);

    vec3 viewDirection = normalize(cameraPos - inPosition);
    vec3 reflectionDirection = reflect(-lightDirection, viewDirection);
    float specularAmount = pow(max(dot(viewDirection, reflectionDirection), 0), 8);
    float specular = specularIntensity * specularAmount;

    return albedo * light.color.rgb * (diffuse + specular) * intensity;
}

vec3 DirectionalLight(vec3 albedo, Light light, vec3 cameraPos) {
    vec3 lightDirection = normalize(light.position.xyz);
    float diffuse = max(dot(inNormal, lightDirection), 0.f);

    vec3 viewDirection = normalize(cameraPos - inPosition);
    vec3 reflectionDirection = reflect(-lightDirection, viewDirection);
    float specularAmount = pow(max(dot(viewDirection, reflectionDirection), 0), 8);
    float specular = specularIntensity * specularAmount;

    return albedo * light.color.rgb * (diffuse + specular);
}

vec3 SpotLight(vec3 albedo, Light light, vec3 cameraPos) {
    float outerCone = 0.9f;
    float innerCone = 0.95f;

    vec3 lightDirection = normalize(light.position.xyz - inPosition);
    float diffuse = max(dot(inNormal, lightDirection), 0.f);

    vec3 viewDirection = normalize(cameraPos - inPosition);
    vec3 reflectionDirection = reflect(-lightDirection, viewDirection);
    float specularAmount = pow(max(dot(viewDirection, reflectionDirection), 0), 8);
    float specular = specularIntensity * specularAmount;

    float angle = dot(vec3(0.f, -1.f, 0.f), -lightDirection);
    float intensity = clamp((angle - outerCone) / (innerCone - outerCone), 0.f, 1.f);

    return albedo * light.color.rgb * (diffuse + specular) * intensity;
}

vec3 Phong(vec3 color) {
    vec3 cameraPos = camera.position.xyz;
    vec3 normal = normalize(inNormal);

    vec3 total = { 0.f, 0.f, 0.f };

    for (uint i = 0; i < lightsMeta.count; i++) {
        vec3 lightColor = lights[i].color.xyz;
        if (lights[i].flags.x == 0) // point
        {
            total += PointLight(color, lights[i], cameraPos);
        }
        else if (lights[i].flags.x == 1) // directional
        {
            total += DirectionalLight(color, lights[i], cameraPos);
        }
        else if (lights[i].flags.x == 2) // spot
        {
            total += SpotLight(color, lights[i], cameraPos);
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
        color = vec4(0.7f, 0.7f, 0.7f, 1.f); // empty texture
    } else {
        color = vec4(texture(textures[pc.textureID], inUV).rgb, 1.f);
    }

    outColor = vec4(Phong(color.rgb), 1.f);
}